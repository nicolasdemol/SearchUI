# SearchUI 1.2.0 — améliorations communautaires

## Fonctionnalités

- Révision du 19 septembre : recherche par plugin retirée, option « Smart Search »
  retirée du MCM. La recherche intelligente est toujours active : tous les mots
  doivent être présents dans le nom, quel que soit leur ordre. Une saisie vide
  ne lance aucune recherche.
- Filtre des enchantements intégrés : tous, non enchantés, enchantés uniquement.
  Les enchantements ajoutés par le joueur à un exemplaire ne sont pas concernés.
- Préférences MCM enregistrées immédiatement après chaque modification dans
  `Data/SKSE/Plugins/SearchUI.settings.yaml`, restaurées au chargement, sur
  une nouvelle partie et à l'ouverture/affichage du MCM. Le fichier global fait
  référence, pas les propriétés stockées dans la sauvegarde. Fermer le MCM
  n'écrit plus de valeurs potentiellement périmées. Avec MO2, le fichier peut
  être redirigé vers Overwrite ; il est à conserver lors des mises à jour.
  Valeurs validées et remplacement par fichier temporaire ; aucune dépendance
  Papyrus de configuration supplémentaire.
- Comparaison Unicode sans distinction de casse et tolérance aux accents latins
  (`epee` trouve `Épée`). Les caractères chinois, cyrilliques et japonais sont
  conservés. La saisie reste celle d'UIExtensions : aucun ajout d'IME ou de presse-papiers.
- Fin explicite des recherches vides et en erreur, annulation, limitation du
  nombre de requêtes en attente et protection contre les résultats périmés.
- Protection des ouvertures répétées, délai d'attente borné et messages d'erreur.

## Architecture et compatibilité

Namespace C++ `Sample` remplacé par `SearchUI`, en-têtes dans `include/SearchUI`.
Les noms publics Papyrus `SearchAPI`, `SearchMCM` et `SearchUIController` et les
signatures natives historiques sont conservés. Les anciens paramètres de filtre
par plugin et de mode exact sont ignorés ; les anciens getters de plugin renvoient
une chaîne vide. `StartItemSearch` et `SaveSettings` constituent l'API actuelle.
Les propriétés `SmartSearch`/`PluginFilter` ont été retirées du contrôleur.
17 fonctions natives sont enregistrées, adaptateurs historiques compris ; installer
ensemble DLL et trois PEX. Les préférences de schéma 1 restent lisibles, en ignorant
`smart` et `plugin`. L'enregistrement passe au schéma 2 sans ces deux champs.
L'ESP existant est conservé à l'identique.

L'index copie les noms, FormIDs, catégories et enchantements
sur le thread principal après DataLoaded. Le worker ne manipule plus de `RE::*`.
Les publications sont protégées par mutex et numéro de génération. Le chargement
d'une sauvegarde annule les recherches en cours. L'index est trié une seule fois ;
l'ordre affiché par le menu de conteneur reste sous le contrôle du jeu/SkyUI.
Les changements de noms/enchantements effectués par d'autres mods après cette
indexation ne sont pas réindexés automatiquement.

CommonLibSSE-NG **8.1.0 reste inchangée** pour ce lot, commit épinglé
`3c0f5a87c3b166c9a6712d5c3bd180e9ac5ad0fd`. La migration préalable était
3.7.0 → 8.1.0. Déclaration NG avec Address Library v5, cibles SE/AE/VR conservées,
sans restriction exclusive à 1.7.104 ni minimum SKSE global qui bloquerait 1.6.1170.

Aucun patch-site, trampoline ou offset ajouté. L'ancien abonnement aux événements
d'ouverture/fermeture de menus n'est plus nécessaire à la publication des résultats.
Les accès au fichier d'origine `GetFile(0)` et `GetFilename()` ont été supprimés.
Le filtre d'enchantement utilise `As<TESEnchantableForm>()` et son membre
`formEnchanting` déclaré par CommonLib.
Ce dernier est lu pendant l'indexation, sans calcul d'offset local.

## Validation

- Reconstruction complète Release x64 réussie, MSVC/Ninja, CommonLib comprise
  (529 étapes, aucun warning). Journal : `build/community-release-build.log`.
- Recompilation Release x64 après la simplification du 19 septembre : réussie.
- Suite CTest réussie, 59 vérifications : recherche, Unicode, catégories/enchantements,
  absence de résultats, rejets de résultats périmés, annulation, concurrence,
  persistance YAML, migration des anciennes préférences, relecture par un lecteur
  ayant d'anciennes valeurs, redémarrage sans sauvegarde de jeu et fichier absent.
  Les trois modes d'enchantement sont relus avec des clés de casse différente.
- Correction du retour possible de « Enchanted items » à « All » : les clés
  reçues depuis Papyrus sont maintenant comparées sans distinction de casse.
  `BSFixedString` peut renvoyer `Enchantment` pour le littéral `enchantment` ;
  une comparaison C++ sensible à la casse retombait alors sur la valeur par défaut.
- Compilation des trois scripts avec le compilateur Papyrus officiel :
  zéro erreur et zéro warning.
- Contrôle du PE x64 et des exports/métadonnées SKSE multi-runtime.
- Probe hors jeu sur l'exécutable 1.7.104 et sa bibliothèque v5 ; modules
  synthétiques 1.6.1170 et 1.5.97 utilisant leurs véritables Address Libraries.

Ces vérifications ne constituent pas un test de chargement SKSE ou de menu en jeu.
Les familles SE/AE/VR restent des cibles de compilation ; VR et les autres
versions n'ont pas été validées en jeu dans ce lot.

## Recette en jeu avant publication

1. Installer ensemble ESP, DLL et les trois PEX avec SkyUI et UIExtensions.
2. Charger par SKSE ; vérifier runtime/version, `registered 17 native functions`
   et `Search index ready` dans `SearchUI.log`.
3. Ouvrir avec F4 (ou touche configurée), rechercher, fermer/réouvrir, puis tester
   une recherche sans résultat et plusieurs pressions rapides sur la touche.
4. Vérifier l'absence des deux options supprimées, chercher des mots dans un ordre
   différent du nom de l'objet, puis essayer une saisie vide, les trois modes
   d'enchantement, catégories et seuil de résultats.
5. Modifier le MCM, le fermer, recharger puis démarrer une autre partie pour
   vérifier les préférences. Tester aussi une mise à jour depuis une sauvegarde
   existante ayant Smart Search désactivé et un filtre plugin renseigné : aucun
   des anciens réglages ne doit agir. Tester un chargement pendant une recherche.
   Modifier aussi un réglage puis changer de page MCM : le fichier doit déjà être
   à jour sans sauvegarder la partie. Les messages `Global preferences saved`
   et `Global preferences loaded` donnent le chemin et les valeurs appliquées.
6. Vérifier l'ajout au conteneur, les quantités, les noms localisés et les objets
   issus de plugins légers ; répéter sur 1.6.1170 et VR si ces builds sont publiés.

## Fichiers

- `src/Search.cpp`, `include/SearchUI/Search.h` : index et accès au moteur.
- `src/SearchCore.cpp`, `include/SearchUI/SearchCore.h` : filtres et cycle de recherche.
- `src/AsyncSearch.cpp`, `include/SearchUI/AsyncSearch.h` : worker et annulation.
- `src/Preferences.cpp`, `include/SearchUI/Preferences.h` : stockage YAML.
- `src/Papyrus.*`, `src/Main.cpp`, `src/Config.*`, `src/PCH.h` : namespace,
  natives, cycle SKSE et logs de diagnostic (niveau info par défaut).
- `Scripts/Source/*.psc` : API, contrôleur et options MCM.
- `CMakeLists.txt`, `vcpkg.json`, `test/Search.cpp`, `tools/Build-Papyrus.ps1` :
  version 1.2.0, compilation et tests.
- `README.md`, ce rapport et note dans l'audit historique : documentation.
