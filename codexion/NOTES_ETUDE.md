# Codexion — notes de conformité

Le sujet `src/en.subject.pdf` impose notamment :

- un thread par coder, un dongle entre chaque paire de voisins (un seul
  dongle pour un seul coder) ;
- la protection de l'état de chaque dongle par mutex, un cooldown après
  libération, et une arbitrage FIFO ou EDF ;
- une heap de priorité pour les demandes FIFO/EDF ;
- un thread monitor séparé, des logs synchronisés et l'arrêt au burnout ou
  lorsque tous les coders ont atteint le nombre de compilations demandé ;
- aucune variable globale, un Makefile à la racine du dépôt et un README
  anglais conforme aux sections requises.

## Choix d'implémentation

- Chaque dongle contient son propre min-heap. La capacité de deux demandes
  découle de la topologie : un dongle est partagé par les deux coders qui
  l'entourent, et n'a qu'un seul utilisateur dans le cas d'un seul coder.
- FIFO priorise l'arrivée la plus ancienne. EDF priorise la deadline de
  burnout la plus proche ; l'arrivée, puis l'identifiant du coder, assurent un
  ordre déterministe en cas d'égalité.
- Les coders prennent les dongles par indices croissants afin d'éviter
  l'attente circulaire.
- Les mutex des dongles protègent aussi leur heap et leur condition variable.
  Les autres mutex protègent les données des coders, l'arrêt et les logs.
- Le parser n'accepte que les schedulers `fifo` et `edf`, comme demandé par le
  sujet. Les arguments numériques sont contrôlés avant leur conversion vers
  les champs entiers.
- Le README à la racine documente compilation, exécution, ressources, usage de
  l'IA, blocages traités et synchronisation des threads.

## Points de validation manuelle

- Vérifier que le login de la première ligne de `../README.md` est celui à
  utiliser pour le rendu et ajouter les logins des éventuels co-auteurs.
- Après toute modification, reconstruire avec `make re` et tester FIFO, EDF,
  les entrées incorrectes et le cas d'un seul coder.
