# Migration Skyrim 1.7.104 / SKSE 2.3.1

> Audit historique de la migration, avant les améliorations 1.2.0.
> Les références à `Sample`, aux sept natives, aux résultats partagés et au sink
> de menus décrivent cet état antérieur. Voir [le rapport 1.2.0](community-1.2.0.md)
> pour l'architecture, les fichiers et la recette actuels.

## Audit avant modification — 18 septembre 2026

Révision SearchUI initiale : `5fa543efdae594b05faf55ce4c47d2648f2a2bb4` (`v1.1.3`).
La version interne CMake/DLL est 1.0.0 ; elle est conservée.

- Build : CMake, presets Ninja/MSVC ou clang-cl, C++23, vcpkg et triplet
  `x64-windows-skse` (bibliothèques statiques, CRT dynamique).
- CommonLib réellement utilisée : **3.7.0**, attestée par
  `build/release-msvc/vcpkg_installed/vcpkg/status`, fournie par le registre
  colorglass au baseline `6309841a1ce770409708a67a9ba5c26c537d2937`.
  Le checkout voisin `../CommonLibSSE-NG`, révision `b93280e`, n'était pas
  la dépendance sélectionnée par ce build.
- Déclaration : `add_commonlibsse_plugin` génère `__SearchUIPlugin.cpp`,
  `SKSEPlugin_Version` et `SKSEPlugin_Query`. `src/Main.cpp` définit
  `SKSEPlugin_Load`. Structure indépendante, Address Library, aucun minimum
  SKSE global ni liste explicite de runtimes. Cibles historiques SE/AE/VR.
- Aucun usage de `REL::ID`, `REL::Relocation`, `VariantID`, `VariantOffset`,
  adresse absolue, `base + offset`, `reinterpret_cast`, patch de vtable,
  `write_call`, `write_branch`, trampoline, taille de structure supposée,
  `IsAE`/`IsSE` ou test de runtime dans le code de production initial.
  `PCH.h` contenait seulement les littéraux REL et un formatter de version.
- `test/Search.cpp` était un squelette sans cas de test. Les scripts et ESP
  présents dans `contrib` sont des restes du template ; l'interface réellement
  installée utilise `SearchUI.esp`, `SearchAPI.pex`, `SearchMCM.pex`,
  `SearchUIController.pex`, SkyUI et UIExtensions, extérieurs au dépôt.

### Inventaire exhaustif des contacts avec le moteur

| Fichier | Utilisations | Vérification / adaptation |
|---|---|---|
| `src/Main.cpp` | Interfaces SKSE de messaging, Papyrus et tâches UI ; `UI::GetSingleton`, `UI::AddEventSink<MenuOpenCloseEvent>`, sous-classe `BSTEventSink` | L'abonnement à DataLoaded constitue le seul « hook ». Aucun patch-site machine. CommonLib possède le layout du gestionnaire et des listes de sinks. Ajout de contrôles de disponibilité et de logs. |
| `src/Search.cpp` | `TESDataHandler::GetSingleton`, `GetFormArray<T>`, itération `BSTArray`, `GetName`, `GetFormID`, `LookupByID`, `As<TESBoundObject>`, `GetFormType`, `AddObjectToContainer` | APIs CommonLib conservées. Aucun accès direct aux champs des objets du jeu. Les RTTI, layouts des formes et fonctions virtuelles sont ceux de CommonLib 8.1.0. |
| `src/Papyrus.cpp` | `IVirtualMachine::RegisterFunction`, `BSFixedString::c_str`, `TESForm::LookupByID`, paramètres `TESObjectREFR*` | Sept signatures SearchAPI inchangées. Contrôle VM nulle et log de fin d'enregistrement. |
| `src/AsyncSearch.cpp`, `include/Sample/AsyncSearch.h` | Thread et files C++ ; appel à la recherche | Aucun accès moteur supplémentaire. Thread déplacé après les membres qu'il utilise, pour empêcher son démarrage avant leur construction. |
| `src/Config.*`, `src/PCH.h`, autres en-têtes | YAML, logger, déclarations | Formatter `fmt::formatter<REL::Version>` local supprimé : fourni désormais par CommonLib. |

Les seules lectures directes de champs RE ajoutées sont `MenuOpenCloseEvent::menuName`
et `opening`, pour le logging. Pas de nouvel offset ni de hook natif.
`AddObjectToContainer` est un appel virtuel déclaré par CommonLib (slot 0x5A dans
son en-tête), **pas** un remplacement de vtable. Son fonctionnement en jeu reste
à valider ; une résolution d'adresse hors jeu ne prouve pas un appel moteur.

## Dépendance et APIs migrées

Nouvelle dépendance : [alandtse/CommonLibSSE-NG 8.1.0](https://github.com/alandtse/CommonLibSSE-NG/releases/tag/v8.1.0),
commit épinglé `3c0f5a87c3b166c9a6712d5c3bd180e9ac5ad0fd` dans
`cmake/CommonLib.cmake`. Compilation depuis les sources via FetchContent,
OpenVR conservé. Le vieux port CommonLib et les dépendances inutilisées
articuno/ryml ne sont plus utilisés. Baseline vcpkg aligné sur CommonLib :
`ee12231b20c95013c6638d845d04c91559a1d1ff`.

Les changements amont nécessaires comprennent :

- reconnaissance des versions mineures >= 6 comme AE (donc aussi 1.7.x) ;
- lecture des Address Libraries formats 1, 2 et 5 ;
- layouts/accessors et RTTI actualisés pour les versions 1.7.x ;
- drapeau Address Library v5 dans `SKSEPluginInfo`, corrigé en 8.0.1.

SearchUI conserve `add_commonlibsse_plugin(USE_ADDRESS_LIBRARY)` et laisse
CommonLib sélectionner le runtime. Aucune restriction à 1.7.104, aucun minimum
SKSE 2.3.1 imposé aux anciennes versions. SKSE et Address Library doivent toujours
correspondre au runtime installé. La DLL vise toujours SE 1.5.x, AE 1.6.x/1.7.x et
VR 1.4.15 ; cette déclaration n'est pas une certification en jeu de toutes ces versions.

`SKSE::Init(skse, SKSE::InitInfo{.log = false})` préserve le logger déjà créé par
SearchUI ; la nouvelle API initialise autrement son propre logger par défaut.
La cible YAML utilise désormais `yaml-cpp::yaml-cpp`. Les sources sont compilées
en UTF-8 afin de préserver les caractères accentués du moteur de recherche.

La licence amont de CommonLib a changé : consulter les fichiers `LICENSE` et
`EXCEPTIONS.md` du commit épinglé avant de redistribuer le binaire. La licence
Apache-2.0 existante de SearchUI n'a pas été réécrite dans cette migration.

## Build reproductible

Dans un Developer PowerShell VS 2022 x64, avec `VCPKG_ROOT` configuré :

```powershell
# Éviter le déploiement automatique dans un profil de jeu pendant la validation.
$env:SkyrimPluginTargets = ''
cmake --preset build-release-msvc -B build/release-1.7.104 -DBUILD_RUNTIME_PROBE=ON
cmake --build build/release-1.7.104 --config Release --clean-first --parallel 6
```

`BUILD_TESTS` est désactivé dans les presets normaux : le squelette Catch2 initial
ne contient aucun test. Pour des tests Catch2 ultérieurs, activer simultanément
`BUILD_TESTS` et la feature vcpkg `tests`. La feature `scripts-dev` reste disponible
mais n'est pas requise pour construire la DLL.

Build local : MSVC 19.44.35213, CMake 4.2.3, Ninja, Release x64 ; CommonLib compilée
depuis les sources dans un nouveau dossier. Le checkout épinglé local a été fourni
par `FETCHCONTENT_SOURCE_DIR_COMMONLIBSSE`; aucun ancien objet CommonLib n'est réutilisé.
La copie de distribution est `contrib/Distribution/PluginRelease/SearchUI.dll`.

Résultat final : **clean build réussi, 523 étapes, aucun warning ni erreur de
compilation/link**. Log : `build/build-release-clean.log`. Les deux copies de la
DLL sont identiques, SHA-256 :
`959b6fe74d407d91ff024e3d1195dec17b14f9def4767f29ee6ec01367b89ad1`.

## Vérifications exécutables

```powershell
python test/verify_plugin.py build/release-1.7.104/SearchUI.dll
& ./build/release-1.7.104/SearchUIRuntimeProbe.exe '<SkyrimSE.exe>' '<dossier des fichiers versionlib>'
```

Le script Python lit le PE sans exécuter la DLL : AMD64, exports Load/Query/Version,
`versionIndependenceEx = 3` (structure indépendante + v5), `versionIndependence = 1`
(Address Library), minimum SKSE = 0. La table de versions est ignorée par SKSE
lorsque le flag Address Library est actif ; CommonLib initialise ses cases
inutilisées à 1.0.0. Référence : [chargeur officiel SKSE](https://github.com/ianpatt/skse64/blob/master/skse64/PluginManager.cpp).

Le probe C++ utilise les APIs de test de **la CommonLib réellement liée**, mappe
le véritable SkyrimSE.exe 1.7.104 installé, vérifie sa classification AE et charge
le fichier `versionlib-1-7-104-0.bin` installé. Il contrôle les globals utilisés par
SearchUI et leur appartenance à la section de données de l'exécutable.
Il charge également les véritables bibliothèques 1.6.1170 et 1.5.97 ; faute de leurs
exécutables, le module de ces deux tests est synthétique. Il ne démarre pas le jeu.

| Global CommonLib | ID SE / AE | RVA 1.7.104 | RVA 1.6.1170 | RVA 1.5.97 |
|---|---|---|---|---|
| TESDataHandler | 514141 / 400269 | 0x219DED8 | 0x20F6320 | 0x1EBE428 |
| UI | 514178 / 400327 | 0x219E5C0 | 0x20F6A00 | 0x1EBEB20 |
| TESForm::allForms | 514351 / 400507 | 0x21A3808 | 0x20FBB88 | 0x1EC3CB8 |
| TESForm::allFormsMapLock | 514360 / 400517 | 0x21A3C98 | 0x20FC018 | 0x1EC4150 |

Ces RVAs sont des **résultats de validation**, jamais des offsets de production.
Ils ne valident pas à eux seuls la sémantique de tous les objets/RTTI/virtuelles.

## Limites et tests en jeu restants

Les deux vérifications ci-dessus ont réussi après le clean build final
(`build/plugin-metadata.log`, `build/runtime-probe.log`). Le chargement de la DLL
par Windows `LoadLibrary` a également réussi ; ses imports sont résolus avec les
DLL système et le runtime MSVC, sans dépendance DLL fmt/spdlog/yaml/OpenVR.
Ce chargement hors jeu **n'appelle pas** `SKSEPlugin_Load` et ne prouve donc pas
que SKSE initialise le plugin dans Skyrim.

Lors de cette première validation, Skyrim 1.7.104, SKSE 2.3.1 et Address Library
1.7.104 étaient disponibles. La validation de la migration initiale s'est limitée
aux contrôles hors jeu ; elle ne constituait pas un test de chargement SKSE.
Des sessions de développement 1.2 ont ensuite confirmé le chargement, l'indexation
et des recherches sous 1.7.104. Consulter le README pour l'état de validation actuel.

À effectuer dans un profil de test avec SearchUI.esp/scripts, SkyUI et UIExtensions :

1. Démarrer via SKSE 2.3.1 et vérifier `loaded correctly` dans `skse64.log`.
2. Vérifier dans `SearchUI.log` le runtime, l'initialisation SKSE, les sept fonctions
   Papyrus, DataLoaded et `MenuOpenCloseEvent sink registered`.
3. Ouvrir la saisie par le raccourci configuré (F4 par défaut), chercher un objet,
   vérifier les résultats et le remplissage du conteneur.
4. Fermer/réouvrir et répéter la recherche plusieurs fois, y compris sans résultat.
5. Répéter sur une installation 1.6.1170 ; SE/VR restent également à tester en jeu.

L'audit a aussi identifié des risques préexistants hors migration : résultats
globaux partagés sans verrou entre worker et Papyrus ; `IsSearchFinished` assimile
un résultat vide à une recherche inachevée. Leur comportement n'a pas été refondu.
Le réordonnancement de construction du worker corrige uniquement sa course de démarrage.

## Fichiers modifiés / ajoutés

- `CMakeLists.txt`, `CMakePresets.json`, `cmake/CommonLib.cmake` : dépendance NG
  épinglée, déclaration multi-runtime, build et probe optionnel.
- `vcpkg.json`, `vcpkg-configuration.json` : dépendances/baseline modernisées.
- `src/PCH.h`, `src/Main.cpp`, `src/Papyrus.cpp` : APIs et diagnostic.
- `include/Sample/AsyncSearch.h` : ordre de construction du worker.
- `test/Search.cpp` : include Catch2 actuel ; squelette toujours sans cas.
- `test/RuntimeProbe.cpp`, `test/verify_plugin.py` : validations hors jeu.
- `README.md`, `docs/runtime-1.7.104.md` : migration et limites de validation.

Le design, les scripts installés, les catégories, les règles de recherche et
les raccourcis de SearchUI n'ont pas été modifiés.
