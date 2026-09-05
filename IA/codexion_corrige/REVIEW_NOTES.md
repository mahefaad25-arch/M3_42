# Codexion — Notes de relecture (à étudier avant la soutenance)

## 1. Bug corrigé : segfault au-delà de 64 coders (CRITIQUE)

**Fichier concerné :** `dongle_waiters.c`

**Avant :**
```c
# define MAX_WAITERS 64
static t_waiter        g_waiters[MAX_WAITERS][MAX_WAITERS];
static int             g_waiters_count[MAX_WAITERS];
static pthread_mutex_t g_waiters_lock = PTHREAD_MUTEX_INITIALIZER;
```

Le sujet dit : *"There is one dongle between each pair of coders"* → il y a **autant de
dongles que de coders** (`nb_coders`). Or `g_waiters` n'avait de la place que pour **64
dongles** (première dimension). Avec `number_of_coders = 6894`, `dongle_id` peut valoir
6893, et `waiter_add()` écrivait alors très loin en dehors du tableau statique →
`Invalid write of size 4` sous Valgrind, puis SIGSEGV. C'est exactement ce que montrait
ton deuxième `valgrind` (ça marchait à 64, plus après).

**Deuxième problème, plus grave pour la correction (Norme / sujet) :** ces trois
variables étaient des **variables globales** (statiques au fichier = globales en C),
alors que le sujet dit noir sur blanc : *"Global variables are forbidden!"*. Même sans
le crash, ce point à lui seul peut valoir un **0** à l'évaluation.

**Correction appliquée :**
- La liste d'attente d'un dongle est maintenant stockée **dans la structure
  `t_dongle` elle-même** (`waiters[MAX_DONGLE_WAITERS]` + `waiters_count`), allouée
  dynamiquement avec le tableau de dongles dans `init.c` (donc taille = `nb_coders`,
  plus aucune limite arbitraire).
- Plus aucune variable globale : la liste est protégée par le mutex du dongle
  (`d->lock`), déjà tenu par tous les appelants (`dongle_try_take`).
- `MAX_DONGLE_WAITERS = 3` (marge de sécurité) suffit largement puisque, par
  construction du sujet, **au plus 2 coders** (les deux voisins) peuvent attendre le
  même dongle en même temps.
- Les prototypes de `waiter_add` / `waiter_remove` prennent maintenant `t_sim *sim`
  pour accéder à `sim->dongles[dongle_id]` au lieu de variables globales.

**Fichiers modifiés :** `codexion.h`, `dongle_waiters.c` (réécrit), `dongle.c` (appels
+ init de `waiters_count`).

---

## 2. Bug corrigé : cooldown "fantôme" au démarrage

**Fichier concerné :** `dongle.c`

`free_since_ms` valait `0` à l'initialisation. Au tout début de la simulation,
`now` vaut aussi ~0, donc `(now - free_since_ms) >= dongle_cooldown` était souvent
**faux** si `dongle_cooldown > 0` : les dongles semblaient "en cooldown" alors qu'ils
n'ont jamais été relâchés. Cela retardait artificiellement le tout premier
`has taken a dongle` de chaque run.

**Correction :** `free_since_ms` est initialisé très loin dans le passé
(`-1000000000`), garantissant que le premier `dongle_try_take()` n'est jamais bloqué
par un cooldown qui n'a pas de sens avant la première utilisation.

---

## 3. Correction Norme : `utils.c` avait 6 fonctions

La Norme 42 limite à 5 fonctions par fichier `.c`. `utils.c` en avait 6
(`time_diff_ms`, `get_timestamp_ms`, `log_state`, `is_stopped`, `set_stop`,
`ft_usleep_ms`). J'ai déplacé `ft_usleep_ms` dans un nouveau fichier `timing.c`
(ajouté au `Makefile`). Pense à relancer `norminette` toi-même pour vérifier le
reste (je n'ai fait qu'une relecture manuelle, pas passé l'outil officiel).

---

## 4. Point à trancher toi-même : la "priority queue (heap)"

Le sujet dit explicitement (chapitre VI) :

> *"You must implement a priority queue (heap) for FIFO/EDF scheduling (C89 has no
> standard library for this)."*

Le code actuel (`find_best_waiter`) fait un **simple scan linéaire** sur la petite
liste d'attente d'un dongle (au plus 2-3 éléments). Fonctionnellement c'est
strictement équivalent à un tas de taille ≤ 3 (le coût algorithmique est négligeable
vu la taille), mais ce n'est **pas littéralement un tas (heap)**.

Je n'ai **pas** réécrit cette partie — à toi de juger si ton évaluateur/correcteur
va être strict sur la structure de données exacte demandée, ou s'il accepte une
implémentation fonctionnellement équivalente vu la taille bornée à 2-3 éléments.
Si tu veux, je peux t'implémenter un vrai tas binaire (binary heap) pour respecter
la lettre du sujet — dis-moi.

---

## 5. Vérifié et jugé conforme (pas de changement)

- **Makefile** : règles `NAME`, `all`, `clean`, `fclean`, `re` présentes, `cc` avec
  `-Wall -Wextra -Werror -pthread`, pas de relink inutile (dépendance sur `.o`
  individuels + `codexion.h`).
- **Fonctions externes utilisées** : toutes dans la liste autorisée du sujet
  (`pthread_*`, `gettimeofday`, `usleep`, `malloc`, `free`, `printf`, `fprintf`,
  `strcmp`).
- **Un seul dongle si un seul coder** (`init_dongles` alloue `nb_coders` dongles,
  donc 1 dongle si `nb_coders == 1`) — conforme au sujet.
- **Ordre d'acquisition des dongles** (`take_both_dongles`) : toujours le plus petit
  index en premier → évite l'attente circulaire (une des 4 conditions de Coffman),
  donc pas de deadlock classique "dîner des philosophes".
- **Format des logs** : `has taken a dongle` (x2) → `is compiling` → `is debugging`
  → `is refactoring`, conforme à l'exemple du sujet.
- **Précision du burnout** : le monitor boucle toutes les 1 ms
  (`usleep(1000)`), largement dans la marge de 10 ms exigée par le sujet.
- **Rejet des arguments invalides** (négatifs, non-entiers, scheduler autre que
  `fifo`/`edf`) : géré par `parsing.c`.

---

## 6. Ce que je te conseille de faire toi-même avant la défense

1. Faire tourner `norminette` sur l'ensemble du projet (je n'ai fait qu'une relecture
   manuelle, pas l'outil officiel).
2. Tester `valgrind --tool=helgrind` (détection de race conditions), pas seulement
   memcheck — utile vu que le projet est très concurrentiel.
3. Tester le mode `edf` en plus de `fifo` (je n'ai testé que `fifo` ici).
4. Être capable d'expliquer précisément **pourquoi** les anciennes variables
   globales posaient un double problème (Norme + robustesse/scalabilité), c'est
   typiquement le genre de question posée en soutenance.
5. Réfléchir à la question du "heap" (§4 ci-dessus) et avoir un avis argumenté à
   présenter à l'évaluateur si la question vient.
