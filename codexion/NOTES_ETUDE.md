# Codexion — Rapport de vérification & notes d'étude

Ce document résume tout ce qui a été vérifié par rapport au sujet, le bug qui
faisait planter `valgrind ./codexion 6894 543 3 2 8 4 4 fifo`, et un problème
plus grave découvert en le cherchant (violation d'une règle **obligatoire**
du sujet). Les diffs exacts sont dans `diff_*.patch` à côté de ce fichier.

---

## 1. LE BUG QUI FAISAIT PLANTER (cause du SIGSEGV)

Fichier : `dongle_waiters.c`

```c
# define MAX_WAITERS 64
static t_waiter g_waiters[MAX_WAITERS][MAX_WAITERS];
static int      g_waiters_count[MAX_WAITERS];
```

`MAX_WAITERS` sert ici à la fois pour :
- la 1re dimension du tableau → indexée par **`dongle_id`**
- la 2e dimension → indexée par le nombre de coders qui attendent CE dongle

Or, `dongle_id` va de `0` à `nb_coders - 1` (il y a autant de dongles que de
coders, cf. sujet chap. VI). Donc dès que `nb_coders > 64`, `waiter_add()`
écrit dans `g_waiters[dongle_id][...]` avec `dongle_id >= 64` → écriture hors
tableau.

- À N=64 : ça passe pile (indices 0..63).
- À N=100 : ça déborde mais reste dans une zone mémoire encore "proche" (BSS),
  donc ça corrompt silencieusement autre chose sans planter tout de suite
  (vérifié : le programme tourne "normalement" mais c'est un comportement
  indéfini).
- À N=6894 : ça sort complètement de la mémoire mappée → `SIGSEGV`, exactement
  ce que Valgrind a rapporté (`Address ... is not stack'd, malloc'd or
  (recently) free'd`).

### Correction

Le vrai nombre de "voisins" qui peuvent attendre un même dongle est **borné
à 2** par la topologie du sujet (chaque dongle est le "dongle gauche" d'un
coder et le "dongle droit" de son voisin, jamais plus). Le tableau
`g_waiters[64][64]` n'avait donc aucune raison d'exister avec ces tailles :
la bonne dimension pour "nombre de dongles" est `nb_coders` (dynamique), pas
une constante 64.

---

## 2. PROBLÈME PLUS GRAVE TROUVÉ EN CORRIGEANT : variables globales

> *"Global variables are forbidden!"* — sujet, Chapitre V, Règles globales.

Les tableaux `g_waiters`, `g_waiters_count` et le mutex `g_waiters_lock`
étaient déclarés `static` **au niveau fichier** dans `dongle_waiters.c`.
En C, `static` au niveau fichier ne rend pas une variable "locale" : ça la
rend juste invisible depuis les autres fichiers (linkage interne), mais
c'est **toujours une variable globale** au sens du langage (elle existe
pendant toute la durée du programme, en dehors de toute pile d'appel).

C'est le genre d'erreur qui, si un correcteur/Deepthought la détecte, peut
valoir un **0** au projet (règle listée en toutes lettres dans le sujet).

### Correction

Suppression totale de l'état global. La liste d'attente de chaque dongle vit
maintenant **à l'intérieur de sa structure `t_dongle`** :

```c
typedef struct s_dongle
{
    ...
    t_waiter    waiters[MAX_DONGLE_WAITERS]; // MAX_DONGLE_WAITERS = 2
    int         waiters_count;
}   t_dongle;
```

Comme `dongle_try_take()` tient déjà `d->lock` pendant tout l'appel à
`waiter_add`/`waiter_remove`/`is_my_turn` (le lock n'est relâché brièvement
que par `pthread_cond_timedwait` à l'intérieur de `short_timed_wait`), on n'a
même plus besoin du mutex global `g_waiters_lock` : le lock du dongle protège
déjà tout, correctement.

Bénéfice supplémentaire : la mémoire utilisée est maintenant `O(nb_coders)`
(un `t_dongle` de plus par coder) au lieu de `O(64 × 64)` fixe qui débordait,
et ça marche pour n'importe quel `nb_coders`, pas seulement jusqu'à 64.

---

## 3. AUTRE BUG TROUVÉ : cooldown appliqué au tout premier prélèvement

Fichier : `dongle.c`, fonction `dongle_init` :

```c
d->free_since_ms = 0;
```

et dans `can_take_now` :

```c
(now - d->free_since_ms) >= sim->p.dongle_cooldown
```

Au tout début de la simulation, `now` est proche de `0` (timestamp relatif au
démarrage) et `free_since_ms` vaut `0` aussi → `now - free_since_ms ≈ 0`.
Comme `dongle_cooldown` est obligatoirement **strictement positif** (vérifié
dans `parsing.c`), la condition `>= dongle_cooldown` est **fausse** à t≈0.

Concrètement : **aucun coder ne peut prendre un dongle avant que
`dongle_cooldown` millisecondes se soient écoulées depuis le lancement du
programme**, alors que le cooldown ne doit s'appliquer qu'*après une
libération* (cf. sujet : *"after a coder releases a dongle, the dongle cannot
be taken again until dongle_cooldown milliseconds have elapsed"* — rien n'est
dit sur le tout début). L'exemple du sujet montre d'ailleurs
`0 1 has taken a dongle` dès `t=0`.

### Correction

```c
d->free_since_ms = -1000000000; // sentinelle "très loin dans le passé"
```

Un dongle qui n'a jamais été relâché ne doit jamais être considéré "en
cooldown". Vérifié après correction : `0 2 has taken a dongle` apparaît bien
dès `t=0` dans les tests.

---

## 4. Ce qui a été vérifié et qui est déjà CORRECT

- **Un thread par coder** (`pthread_create` dans `main.c`) ✔
- **Un dongle par coder**, dongle unique si `nb_coders == 1` (`init.c`) ✔
- **Ordre fixe d'acquisition (min index d'abord)** dans
  `take_both_dongles()` → évite l'attente circulaire (une des 4 conditions
  de Coffman), donc pas de deadlock classique "dining philosophers" ✔
- **Cooldown** appliqué correctement après une libération (`dongle_release`
  met à jour `free_since_ms`) ✔ (seul le cas "avant la 1re prise" était buggé,
  cf. §3)
- **FIFO** : le plus ancien `arrival_ms` gagne ✔
- **EDF** : la deadline la plus proche gagne, égalité tranchée par
  `arrival_ms` (empêche la famine) ✔
- **Détection de burnout** par un thread `monitor` séparé, avec un sleep de
  1 ms → largement dans la tolérance des 10 ms exigée ✔
- **Logs sérialisés** par un mutex (`log_lock`) pour ne jamais mélanger deux
  lignes ✔
- **Arrêt de la simulation** soit par burnout, soit quand tous les coders ont
  atteint `number_of_compiles_required` ✔
- **Validation des arguments** : rejette les négatifs, non-entiers, valeurs
  nulles, scheduler invalide ✔
- **Pas de fuite mémoire** : vérifié par AddressSanitizer + LeakSanitizer
  (valgrind indisponible dans cet environnement, dépendance système
  manquante) — aucune fuite, aucun accès mémoire invalide détecté même à
  N=6894, N=100, N=5, N=2, N=1 ✔
- Compile proprement avec `-Wall -Wextra -Werror -pthread`, aucun warning ✔

---

## 5. Points à étudier / discuter avant la soutenance (pas corrigés, à vérifier vous-même)

1. **"You must implement a priority queue (heap) for FIFO/EDF scheduling"**
   (sujet, chap. VI). Le code actuel (avant et après ma correction) utilise
   une simple recherche linéaire (`find_best_waiter`) sur un petit tableau,
   pas un tas (heap). Fonctionnellement le résultat est identique (le tas
   n'a d'intérêt que pour la complexité algorithmique quand il y a
   *beaucoup* d'éléments à trier), et comme il y a au maximum 2 éléments par
   dongle ici, un heap n'apporterait rien en pratique. **Mais le sujet le
   demande explicitement** : si l'évaluateur applique la lettre du sujet,
   vous pourriez perdre des points pour ne pas avoir de vraie structure de
   tas, même si le comportement observable est correct. À vous de juger si
   vous voulez l'implémenter "pour la forme" ou être prêt à défendre le
   choix (le préparer comme réponse de soutenance : "j'ai choisi une liste
   bornée à 2 éléments par dongle car la topologie du sujet garantit qu'on
   n'a jamais plus de 2 concurrents par ressource, un tas n'apporte donc
   aucun bénéfice ici").

2. **Structure de rendu** : le sujet indique *"Files to Submit: Makefile,
   *.c, *.h in directory coders/"*. Dans l'archive originale, tous les
   fichiers étaient à la racine. Je les ai remis dans un dossier `coders/`
   dans l'archive livrée — vérifiez que c'est bien la structure attendue
   par votre dépôt Git.

3. **`parse_positive`** convertit vers `int` avec un simple cast
   (`p->nb_coders = (int)tmp;`) sans vérifier un dépassement de `INT_MAX`.
   Avec un `number_of_coders` absurdement grand (ex: 99999999999), le
   comportement serait indéfini/faux silencieusement. Pas critique pour le
   sujet (les valeurs de test sont raisonnables), mais bon réflexe à
   mentionner si on vous pose la question en soutenance.

4. **README.md** : le sujet exige un README avec sections précises
   (Description, Instructions, Resources incluant l'usage de l'IA,
   "Blocking cases handled", "Thread synchronization mechanisms", 1re ligne
   en italique avec les logins). Ce fichier n'était pas fourni dans les
   sources — pensez à le rédiger avant le rendu, il est obligatoire.

---

## 6. Fichiers modifiés

- `codexion.h` : ajout de `t_waiter`, `MAX_DONGLE_WAITERS`, champs
  `waiters`/`waiters_count` dans `t_dongle`, signatures de
  `waiter_add`/`waiter_remove`/`is_my_turn` changées pour prendre un
  `t_dongle *` au lieu d'un `int dongle_id`.
- `dongle_waiters.c` : réécrit sans aucune variable globale/statique de
  fichier ; la liste d'attente vit dans `t_dongle`.
- `dongle.c` : appels mis à jour (`waiter_add(d, ...)` etc.), correction du
  cooldown initial (`free_since_ms` sentinelle négative).

Aucun autre fichier n'a été modifié. Voir les patchs `diff_*.patch` fournis
pour le détail ligne par ligne.
