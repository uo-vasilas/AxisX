# Axis X

**Axis X** is a GameMaster tool for [Sphere/SphereX](https://spherecommunity.net)-based Ultima Online shards.
It is a modernized, heavily reworked successor of the classic **Axis2** (GPL v2) with a new dashboard UI,
a dark or light design, English/German interface, web-based profiles with local caching, and many
performance and stability fixes.

*Deutsche Anleitung weiter unten. / German instructions below.*

![License: GPL v2](https://img.shields.io/badge/license-GPLv2-blue.svg)

If Axis X is useful to you, you can support its development on
[Ko-fi](https://ko-fi.com/uoschattenwelt) (heart icon in the footer, or *About*). This is voluntary;
the tool stays free and GPL.

---

## English

### What's new in Axis X

- **Interface in English or German.** Choose it on the first page of the installer or later under
  *Settings -> General -> Language*. The default is English.
- **Dashboard with cards** showing client status (running or not), the loaded profile with item/NPC
  counts, the remote console connection, recent events from the log, recently added items, and
  **quick actions you can define yourself**.
- **Dark or light design** (*Settings -> General -> Theme*). Every page, dialog, list, tab and popup follows
  it, including the script viewer and editor with matching syntax colours, list selections and (in the dark
  design) the Windows confirmation prompts. Primary actions are gold buttons.
- **Mini bar when minimized**: a small floating bar with the sidebar icons. Click an icon to bring Axis X back
  on that page, or use the gold button (or double-click the bar) to restore it. It can be moved and remembers its
  position; *Settings -> General* can switch it off, then Axis X minimizes to the taskbar as usual.
- **Create server profile** with one click (profile dialog) - see "Shipping items" below.
- **Resizable, maximizable window.** Pages scale proportionally, so small laptop screens work too. Size
  and maximized state are remembered. The sidebar tightens its rows on low windows instead of scrolling,
  and switching pages shows the new page directly at its final size (no flicker).
- **Reworked pages.** Every page was redesigned into clear groups with tooltips and plain-language hints.
  See the guide below.
- **Commands reach the client reliably.** The command line is cleared before each command, keys are typed
  as Unicode (umlauts work), and the client window is brought to the front first.
- **Petrify instead of "set flags".** The Spawns page petrifies an NPC with `.set stone`, which toggles
  only that state; the old freeze button overwrote all of the NPC's flags (e.g. invulnerable).
- **Safer GM commands.** Destructive commands (Kill, Remove, Nuke) sit in a separate *Careful* area and
  ask for confirmation.
- **Shared search** with categories on the Items, Spawns and Travel pages.
- **Character value editor.** Set skills, stats and other properties from drop-downs, with a hint that
  explains each value's format.
- **Launcher starts ClassicUO.** It detects `ClassicUO.exe` and passes server, port, account and
  password. A missing `uo.cfg` no longer blocks the start.
- **World map fixed.** Statics now show at every zoom level and coastlines render correctly. The
  background and the map edges are dark.
- **Log page** with a level filter, a text filter, copy and clear.
- **Server profile without scripts.** Players and GMs without the shard's script folder get items and
  NPCs from a profile file in their user folder (`%LOCALAPPDATA%\AxisX\Profile`) or next to `AxisX.exe`.
- **Web profiles with local cache.** Script data comes from your server over HTTP(S)/FTP and is cached
  in `%LOCALAPPDATA%\AxisX\WebCache`. Only newer copies are downloaded, and the cache keeps working
  offline.
- **Custom branding**: your own logo and window icon, from a file or a URL.
- **Remote console** (Sphere remote admin), reachable from the dashboard and the GM Commands page.
- **Update check.** At startup (at most once a day) Axis X asks GitHub for the latest release. If there
  is a newer one, the dashboard shows **Update now**: Axis downloads the installer, checks its SHA-256
  checksum against the one GitHub publishes, starts it and closes. The installer keeps the folder,
  settings and server profiles. Switch it off or check by hand under *Settings -> General*.
- **A single statically linked 64-bit `AxisX.exe`.** No runtime installation is needed.

### Guide to all areas

The sidebar on the left groups the pages. The footer icons at the bottom are, from left to right:
**Settings**, **switch profile**, **About**, **Ko-fi** and **Exit**.

Most buttons send a command to the game client. Buttons marked "click afterwards" then want a target:
click the character or item in the game.

**Overview**
- **Overview (dashboard)**
  - Client and profile status, and recently added items.
  - Quick actions: add your own with a label and a command, and remove them again.
  - Connect to the remote console.

**World**
- **Items**
  - Search all items live by name, defname, graphic ID or category (several words must all match), or
    browse by category. The list shows graphic, category, weight and properties; the preview shows the item.
  - Create the item (double-click works too) or set it as a static.
  - Place what you click next: move it by N tiles in 8 directions, raise/lower, rotate, tile an area at a
    chosen Z height.
  - Set up a spawn point for the item (amount, rate, radius, from/to minutes).
  - Remove the target, or nuke an area (optionally run a command on every item in it instead).
  - Items you added recently also appear on the dashboard.
- **Edit Item**
  - Applies to the item you click next.
  - Change attributes (blessed, newbie, static and so on), type, events, properties, tags and colour
    (with a quick palette).
  - Door and light wizards.
- **Spawns**
  - Search NPCs and items and preview them.
  - Summon an NPC (double-click works too); the preview shows the NPC with its equipment from the scripts.
  - Remove, shrink or **petrify** the NPC you click (petrify toggles; other flags stay).
  - Set a home point and home radius.
  - Place a spawn point and set its amount, from/to minutes and radius.
- **Travel**
  - Search places from the profile and pick them on the large map.
  - Go there, send a player there, or save your own destinations.

**Players**
- **Edit Character**
  - Grouped flags (State, Visibility and magic, Other) that apply to the character you click next.
  - A value editor for skills, stats and properties.
- **Account**
  - Applies to the account of the character you click next.
  - Set the plevel (`xprivset`), account privileges such as jailed, banned and "see all", and the
    client expansion level (resdisp).

**Tools**
- **GM Commands**
  - Commands for yourself: invulnerable, invisible, GM mode, night sight and so on.
  - Target commands: info, edit, link, rotate, shrink, duplicate.
  - Information, discipline, and GM pages (list, queue, go to).
  - Vendors, and weather and light for the current sector.
  - Server commands: save, resync, restock, remote console.
  - The *Careful* area, whose commands ask before running.
- **Custom Commands**
  - Build your own buttons; the *Examples* tab shows ready-made ones to copy.
- **Spells & Sounds**
  - Cast spells, play music and play sounds, each list with a filter.
- **Launcher**
  - Start the default client, or any other client such as ClassicUO.
  - Keep a list of servers with account and password.

**System**
- **Log**
  - All Axis messages, filterable by level (info, warning, error) and text.
  - Copy or clear the log.

**Footer and window**
- **Profile** (swap icon): load, create, edit or export profiles, and **Create server profile**.
- **Ko-fi** (heart) opens the support page, **About** shows version and credits.
- **Minimize** shows the mini bar (see above); the tray icon also brings Axis X back.

**Settings** (gear icon)
- **General**: window behaviour, mini bar, start page, **language**, **theme**, command prefix, client window title, logo
  and icon, and reset buttons.
- **File paths**: UO client and default client.
- **Items**, **Travel** and **Spawns**: display options for these pages.
- **MUL paths**: override individual `*.mul`/`*.uop` files.

### Requirements

- 64-bit Windows 10/11 (Axis X is a 64-bit program).
- An Ultima Online client installation, for the `*.mul`/`*.uop` art, map and sound files.
- A Sphere/SphereX server, if you want the remote console and web profiles.

### Installation

Download `AxisX_Setup_<version>.exe` from the [releases](https://github.com/uo-vasilas/AxisX/releases)
and run it. The first installer page asks for the language; the same language is then used by Axis X.
You can change it later under *Settings -> General*. Later versions arrive through the update check
(or run a newer installer over the old one; it reuses the folder and install mode).

### Building

1. Install **Visual Studio 2022/2026** with the *Desktop development with C++* workload,
   **including MFC** (Individual Components -> "C++ MFC for latest build tools (x86 & x64)").
2. Open `AxisX.sln`, pick `Release | x64`, and build. The binary lands in `Release\AxisX.exe`.
   Alternatively, run `tools\Build-Axis.ps1`.
3. The optional CHM help requires *HTML Help Workshop* (`hhc`) in the PATH.
4. Installer: compile `Setup\AxisX.nsi` with NSIS 3 (`makensis Setup\AxisX.nsi`).

### Configuration

All settings live under `HKEY_CURRENT_USER\Software\Sphere\GM Tools` and are managed from within the
app (footer gear icon -> Settings) unless noted otherwise.

**First start**
- Axis X asks for your UO client and takes the mul path from it. Both can be changed later under
  *Settings -> File paths*; individual files can be overridden under *MUL paths*.
- Pick or create a **profile** (footer swap icon).

**Profiles**
- *Local profile*: select the `.scp` script files, or a whole directory, to load.
- *Web profile*: enter a URL instead. The URL points either directly to a single file, or to a
  directory with an **`AxisSvr.ini`** manifest that lists one relative script filename per line:
  ```
  spheretables.scp
  scripts/sphere_item_ore.scp
  scripts/spherechars.scp
  ```
  - Axis X downloads the manifest and every listed file into the local cache and loads them from
    there. Later starts re-download only changed files.
  - If the server asks for a login (HTTP 401 / Basic Auth), a login dialog appears. Credentials are
    not stored.
- *Load default profile on startup* (Settings -> General), together with a profile marked as default,
  gives a zero-click start.

**Shipping items to players without scripts (for shard operators)**
1. Start Axis X with the real server scripts (a local profile).
2. In the profile dialog, click **Create server profile**. Axis X writes
   `%LOCALAPPDATA%\AxisX\Profile\AxisProfile.scp` - your user folder, no admin rights needed.
3. Pass it on:
   - **Installer:** build with `makensis /DWITH_PROFILE Setup\AxisX.nsi`. This packs the profile files from
     your user folder; the installer puts them into the user folder of whoever installs (or next to
     `AxisX.exe` for an all-users installation). Without the switch no profile is included.
   - **Zip/manual:** put the file into a `Profile` folder next to `AxisX.exe`.

*Export profile* does the same but lets you choose the file name and location.

If no valid script folder is configured, Axis X looks for a profile file first in
`%LOCALAPPDATA%\AxisX\Profile`, then in `Profile` next to `AxisX.exe`, and loads it automatically. The
file contains only what Axis needs to display items (name, defname, graphic, colour, category). The NPC
equipment preview needs the full scripts. See `LIESMICH.txt` in the profile folder.

**Registry values** (`HKCU\Software\Sphere\GM Tools`, all set from within the app)

| Value | Meaning |
|---|---|
| `Language` | `eng` (default) or `deu` |
| `Theme` | `dark` (default) or `light` |
| `DisableToolbar` | 1 = no mini bar, minimize to the taskbar |
| `MiniBar X`, `MiniBar Y` | position of the mini bar |
| `Window Width`, `Window Height`, `Window Maximized` | window size from the last session |
| `DashQuickActions`, `RecentAdds` | dashboard quick actions and recently added items |
| `Staff Prefix` | account prefix stripped from the dashboard greeting (default `+staff_`) |
| `CheckUpdates` | 0 = no update check at startup (default 1) |
| `LastUpdateCheck` | date of the last update check (`YYYYMMDD`) |

**Custom branding (logo and icon)**
- *Settings -> General -> "Logo (file or URL)"*: an image (PNG/BMP/JPG/GIF/ICO) shown in the sidebar
  header.
- *Settings -> General -> "Icon (.ico, file or URL)"*: replaces the window and taskbar icon.
- Both accept a local path or an `http(s)://` URL. URLs are cached in `%LOCALAPPDATA%\AxisX\Branding`.

**Remote console**
- Open it from the dashboard (*Connect*) or from *GM Commands -> Remote Console*.
- A login dialog asks for the server IP, port, account and password of an account with a sufficient
  plevel. Nothing is sent until you confirm it with OK.
- If your shard prefixes staff accounts (e.g. `+staff_name`), set the `Staff Prefix` registry value.

**Language**
- Change it under *Settings -> General -> Language*, or set the `Language` registry value to `eng` or
  `deu`. The change takes effect on the next start.
- Files:
  - `Language\<code>.lng`: message strings.
  - `Language\eng.ui.txt`: translations of the interface texts, one line per text, in the form
    `German<TAB>English`.
- `tools\Extract-Translatable.ps1 -Table Release\Language\eng.ui.txt` lists texts that are still
  missing from the table.

### Developer tools (`tools\`)

| Script | Purpose |
|---|---|
| `Build-Axis.ps1 [-Install -CloseRunning -Start]` | build with MSBuild, copy to `Release\`, start |
| `Release-Axis.ps1 -Version 1.1 [-Makensis <path>] [-Publish]` | set the version, build exe and installer; with `-Publish` commit, tag and create the GitHub release (notes from `release\changelog.txt`) |
| `Capture-AxisWindow.ps1` | screenshot of the Axis window or a popup, optionally after a click |
| `Check-AxisLabels.ps1 [-Width -Height / -Maximize]` | reports labels on all pages whose text does not fit |
| `Check-AxisPopups.ps1` | opens the popup dialogs one by one, checks them and closes them again |
| `Test-AxisMiniBar.ps1` | minimizes Axis and clicks mini-bar icons |
| `Make-AxisIcons.ps1 [-App]` | generates the installer icons and header image (optionally also the program icon) |
| `Extract-Translatable.ps1`, `Wrap-Translatable.ps1` | maintain the translation table |

The check scripts only send window messages to Axis itself (no keystrokes to other windows).

### Future projects

Not planned yet - these are picked up when there is demand:

- **macOS and Linux.** Axis X is built on MFC and Win32 and cannot be ported directly. The main hurdle is
  getting commands into the game: Axis types them into the client window, which works differently on
  macOS/Linux (X11 only, restricted under Wayland, macOS needs accessibility permission). The planned route
  would be a local command channel in the game client (ClassicUO-based) plus a cross-platform Axis in
  C#/.NET, which could reuse ClassicUO's file readers. Running Axis under Wine only helps if the client runs
  under Wine as well.

### License

GPL v2 - see [LICENSE](LICENSE). Based on Axis (Philip A. Esterle/Adron) and Axis2 (Benoit Croussette).

---

## Deutsch

**Axis X** ist ein GameMaster-Tool für Ultima-Online-Shards auf [Sphere/SphereX](https://spherecommunity.net)-Basis.
Es ist ein modernisierter Nachfolger des klassischen **Axis2** (GPL v2) mit neuem Dashboard, dunklem
oder hellem Design, deutscher und englischer Oberfläche, Web-Profilen mit lokalem Cache sowie vielen Performance-
und Stabilitäts-Fixes.

Wer Axis X nützlich findet, kann die Entwicklung auf [Ko-fi](https://ko-fi.com/uoschattenwelt)
unterstützen (Herz-Symbol im Footer oder unter *Über*). Das ist freiwillig; das Tool bleibt kostenlos
und GPL.

### Neu in Axis X

- **Oberfläche auf Deutsch oder Englisch.** Die Sprache wird auf der ersten Seite des Installers oder
  später unter *Einstellungen -> Allgemein -> Sprache* gewählt. Standard ist Englisch.
- **Übersicht mit Karten**: Client-Status (läuft oder nicht), geladenes Profil mit Item- und
  NPC-Zahlen, Verbindung der Remote-Konsole, letzte Ereignisse aus dem Protokoll, zuletzt erzeugte Items
  und **eigene Schnellaktionen**.
- **Dunkles oder helles Design** (*Einstellungen -> Allgemein -> Design*). Alle Seiten, Dialoge, Listen,
  Reiter und Popups folgen ihm, auch Skript-Betrachter und -Editor mit passenden Syntaxfarben, die Auswahl in
  Listen und (im dunklen Design) die Windows-Rückfragen. Hauptaktionen sind goldene Knöpfe.
- **Mini-Leiste beim Minimieren**: eine kleine schwebende Leiste mit den Symbolen der Seitenleiste. Ein Klick
  auf ein Symbol holt Axis X auf dieser Seite zurück, der goldene Knopf (oder ein Doppelklick auf die Leiste)
  vergrößert ohne Seitenwechsel. Sie lässt sich verschieben und merkt sich die Position; unter *Einstellungen
  -> Allgemein* abschaltbar, dann minimiert Axis X ganz normal in die Taskleiste.
- **Server-Profil auf Knopfdruck** (Profil-Dialog) - siehe "Items für Spieler ohne Skripte" unten.
- **Fenster größenveränderbar und maximierbar.** Die Seiten skalieren proportional, das passt auch auf
  Laptops. Größe und Maximiert-Zustand werden gemerkt. Die Seitenleiste rückt bei niedrigen Fenstern
  enger zusammen statt zu scrollen, und ein Seitenwechsel zeigt die neue Seite gleich in fertiger Größe
  (kein Flackern).
- **Überarbeitete Seiten.** Alle Seiten sind neu gegliedert, mit Tooltips und verständlichen Hinweisen
  (siehe Anleitung unten).
- **Befehle kommen zuverlässig im Client an.** Die Befehlszeile wird vor jedem Befehl geleert, Zeichen
  werden als Unicode getippt (Umlaute funktionieren), und das Client-Fenster wird vorher nach vorne geholt.
- **Versteinern statt "set flags".** Die Spawns-Seite versteinert einen NPC mit `.set stone`, das nur
  diesen Zustand umschaltet; der alte Einfrieren-Knopf überschrieb alle Flags des NPCs (z. B. unverwundbar).
- **Sichere GM-Befehle.** Zerstörerische Befehle (Töten, Entfernen, Nuke) liegen im Bereich *Vorsicht*
  und fragen vorher nach.
- **Gemeinsame Suche** mit Kategorien auf den Seiten Items, Spawns und Reisen.
- **Werte-Editor für Charaktere.** Skills, Stats und Eigenschaften werden per Auswahlliste gesetzt;
  ein Hinweis erklärt jeweils das Format.
- **Launcher startet ClassicUO.** Er erkennt die `ClassicUO.exe` und übergibt Server, Port, Konto und
  Passwort. Eine fehlende `uo.cfg` blockiert den Start nicht mehr.
- **Weltkarte korrigiert.** Statics sind in jeder Zoomstufe sichtbar, Küsten werden richtig
  dargestellt, Hintergrund und Kartenränder sind dunkel.
- **Protokoll** mit Filter nach Stufe und Text, Kopieren und Leeren.
- **Server-Profil ohne Skripte.** Spieler und GMs ohne Skriptordner bekommen Items und NPCs aus einer
  Profil-Datei im Benutzerordner (`%LOCALAPPDATA%\AxisX\Profile`) oder neben der `AxisX.exe`.
- **Web-Profile mit lokalem Cache**: Skriptdaten kommen per HTTP(S)/FTP vom Server und werden in
  `%LOCALAPPDATA%\AxisX\WebCache` zwischengespeichert. Neu geladen wird nur, was sich geändert hat;
  offline arbeitet Axis X mit dem Cache weiter.
- **Eigenes Logo und Icon**, aus einer Datei oder von einer URL.
- **Remote-Konsole** (Sphere Remote Admin), erreichbar von der Übersicht und den GM-Befehlen aus.
- **Update-Prüfung.** Beim Start (höchstens einmal am Tag) fragt Axis X bei GitHub nach dem neuesten
  Release. Gibt es ein neueres, zeigt die Übersicht **Jetzt aktualisieren**: Axis lädt den Installer,
  vergleicht seine SHA-256-Prüfsumme mit der von GitHub veröffentlichten, startet ihn und beendet sich.
  Der Installer behält Ordner, Einstellungen und Server-Profile. Abschalten oder von Hand prüfen unter
  *Einstellungen -> Allgemein*.
- **Eine einzige statisch gelinkte 64-Bit-`AxisX.exe`.** Keine Laufzeit-Installation nötig.

### Anleitung zu allen Bereichen

Die Seitenleiste links gruppiert die Seiten. Die Symbole im Footer unten sind von links nach rechts:
**Einstellungen**, **Profil wechseln**, **Über**, **Ko-fi** und **Beenden**.

Die meisten Knöpfe schicken einen Befehl an den Spiel-Client. Bei Knöpfen mit "danach anklicken" folgt
ein Ziel: den Charakter oder das Item im Spiel anklicken.

**Übersicht**
- **Übersicht**
  - Status von Client und Profil, zuletzt erzeugte Items.
  - Schnellaktionen: eigene mit Beschriftung und Befehl anlegen und wieder löschen.
  - Verbindung zur Remote-Konsole.

**Welt**
- **Items**
  - Alle Items live suchen nach Name, Defname, Grafik-ID oder Kategorie (mehrere Wörter müssen alle
    passen) oder nach Kategorie blättern. Die Liste zeigt Grafik, Kategorie, Gewicht und Eigenschaften, die
    Vorschau das Item.
  - Erstellen (auch per Doppelklick) oder als Static setzen.
  - Platzieren, was als Nächstes angeklickt wird: um N Felder in 8 Richtungen bewegen, höher/tiefer,
    drehen, eine Fläche auf einer gewählten Z-Höhe kacheln.
  - Spawnpunkt für das Item einrichten (Anzahl, Rate, Radius, ab/bis Minuten).
  - Ziel entfernen oder eine Fläche nuken (wahlweise statt Löschen einen Befehl auf jedes Item darin).
  - Zuletzt erzeugte Items erscheinen auch in der Übersicht.
- **Item bearbeiten**
  - Wirkt auf das Item, das als Nächstes angeklickt wird.
  - Attribute (gesegnet, newbie, static usw.), Typ, Events, Eigenschaften, Tags und Farbe (mit
    Schnellpalette) ändern.
  - Tür- und Licht-Assistent.
- **Spawns**
  - NPCs und Items suchen und ansehen.
  - NPC beschwören (auch per Doppelklick); die Vorschau zeigt den NPC mit seiner Ausrüstung aus den Skripten.
  - Den angeklickten NPC entfernen, schrumpfen oder **versteinern** (schaltet um; andere Flags bleiben).
  - Zuhause und Heimradius setzen.
  - Spawnpunkt platzieren, mit Anzahl, ab/bis Minuten und Radius.
- **Reisen**
  - Orte aus dem Profil suchen und auf der großen Karte wählen.
  - Hinreisen, Spieler hinschicken oder eigene Ziele speichern.

**Spieler**
- **Charakter bearbeiten**
  - Flags in Gruppen (Zustand, Sichtbarkeit und Magie, Sonstiges); sie wirken auf den Charakter, der
    als Nächstes angeklickt wird.
  - Werte-Editor für Skills, Stats und Eigenschaften.
- **Konto**
  - Wirkt auf das Konto des Charakters, der als Nächstes angeklickt wird.
  - PLevel setzen (`xprivset`), Kontorechte wie eingesperrt, gesperrt und "alles sehen" sowie die
    Client-Stufe (Resdisp).

**Werkzeuge**
- **GM-Befehle**
  - Befehle für sich selbst: unverwundbar, unsichtbar, GM-Modus, Nachtsicht usw.
  - Ziel-Befehle: Info, Bearbeiten, Link, Drehen, Schrumpfen, Duplizieren.
  - Information, Disziplin und GM-Pages (Liste, Warteschlange, Hingehen).
  - Händler sowie Wetter und Licht im aktuellen Sektor.
  - Server-Befehle: Speichern, Resync, Auffüllen, Remote-Konsole.
  - Der Bereich *Vorsicht*: Diese Befehle fragen vor dem Ausführen nach.
- **Eigene Befehle**
  - Eigene Knöpfe bauen; der Reiter *Beispiele* zeigt fertige Vorlagen zum Übernehmen.
- **Zauber & Sounds**
  - Zauber wirken, Musik und Sounds abspielen, jede Liste mit Filter.
- **Launcher**
  - Standard-Client oder einen anderen Client wie ClassicUO starten.
  - Serverliste mit Konto und Passwort.

**System**
- **Protokoll**
  - Alle Meldungen von Axis, filterbar nach Stufe (Info, Warnung, Fehler) und Text.
  - Protokoll kopieren oder leeren.

**Footer und Fenster**
- **Profil** (Tausch-Symbol): Profile laden, anlegen, bearbeiten, exportieren und **Server-Profil erzeugen**.
- **Ko-fi** (Herz) öffnet die Unterstützer-Seite, **Über** zeigt Version und Mitwirkende.
- **Minimieren** zeigt die Mini-Leiste (siehe oben); auch das Tray-Symbol holt Axis X zurück.

**Einstellungen** (Zahnrad)
- **Allgemein**: Fensterverhalten, Mini-Leiste, Startseite, **Sprache**, **Design**, Befehlspräfix, Client-Fenstertitel, Logo
  und Icon sowie Zurücksetzen.
- **Dateipfade**: UO-Client und Standard-Client.
- **Items**, **Reisen** und **Spawns**: Anzeigeoptionen dieser Seiten.
- **MUL-Pfade**: einzelne `*.mul`/`*.uop`-Dateien überschreiben.

### Voraussetzungen

- 64-Bit-Windows 10/11 (Axis X ist ein 64-Bit-Programm).
- Eine Ultima-Online-Client-Installation für die `*.mul`/`*.uop`-Dateien (Grafiken, Karten, Sounds).
- Ein Sphere/SphereX-Server für Remote-Konsole und Web-Profile.

### Installation

`AxisX_Setup_<version>.exe` von den [Releases](https://github.com/uo-vasilas/AxisX/releases) laden und
ausführen. Die erste Seite fragt nach der Sprache; dieselbe Sprache verwendet danach auch Axis X. Später
lässt sie sich unter *Einstellungen -> Allgemein* ändern. Neue Versionen kommen über die Update-Prüfung
(oder einen neueren Installer über den alten laufen lassen; er übernimmt Ordner und Installationsart).

### Bauen

1. **Visual Studio 2022/2026** mit dem Workload *Desktopentwicklung mit C++* installieren,
   **inklusive MFC** (Einzelne Komponenten -> "C++-MFC für die neuesten Buildtools (x86 & x64)").
2. `AxisX.sln` öffnen, `Release | x64` wählen und bauen. Die Exe landet in `Release\AxisX.exe`.
   Alternativ `tools\Build-Axis.ps1` ausführen.
3. Die optionale CHM-Hilfe braucht den *HTML Help Workshop* (`hhc`) im PATH.
4. Installer: `Setup\AxisX.nsi` mit NSIS 3 kompilieren (`makensis Setup\AxisX.nsi`).

### Konfiguration

Alle Einstellungen liegen unter `HKEY_CURRENT_USER\Software\Sphere\GM Tools` und werden in der App
verwaltet (Zahnrad im Footer -> Einstellungen), sofern nicht anders angegeben.

**Erster Start**
- Axis X fragt nach dem UO-Client und übernimmt daraus den Mul-Pfad. Beides lässt sich unter
  *Einstellungen -> Dateipfade* ändern, einzelne Dateien unter *MUL-Pfade*.
- Danach ein **Profil** anlegen oder wählen (Tausch-Symbol im Footer).

**Profile**
- *Lokales Profil*: die `.scp`-Skripte oder ein ganzes Verzeichnis auswählen.
- *Web-Profil*: stattdessen eine URL eintragen. Sie zeigt entweder direkt auf eine Datei oder auf ein
  Verzeichnis mit einer **`AxisSvr.ini`**, die zeilenweise die relativen Skript-Dateinamen auflistet:
  ```
  spheretables.scp
  scripts/sphere_item_ore.scp
  scripts/spherechars.scp
  ```
  - Axis X lädt Manifest und Dateien in den lokalen Cache und liest sie von dort. Bei späteren Starts
    werden nur geänderte Dateien neu geladen.
  - Verlangt der Server einen Login (HTTP 401 / Basic Auth), erscheint ein Login-Dialog.
    Zugangsdaten werden nicht gespeichert.
- *Standardprofil beim Start laden* (Einstellungen -> Allgemein) ergibt zusammen mit einem als
  Standard markierten Profil den Null-Klick-Start.

**Items für Spieler ohne Skripte bereitstellen (für Betreiber)**
1. Axis X mit den echten Server-Skripten starten (lokales Profil).
2. Im Profil-Dialog auf **Server-Profil erzeugen** klicken. Axis X schreibt
   `%LOCALAPPDATA%\AxisX\Profile\AxisProfile.scp` - in den Benutzerordner, ohne Administratorrechte.
3. Weitergeben:
   - **Installer:** mit `makensis /DWITH_PROFILE Setup\AxisX.nsi` bauen. Das packt die Profil-Dateien aus
     deinem Benutzerordner ein; der Installer legt sie in den Benutzerordner dessen, der installiert (bei
     Installation für alle Benutzer neben die `AxisX.exe`). Ohne den Schalter ist kein Profil dabei.
   - **Zip/von Hand:** die Datei in einen Ordner `Profile` neben der `AxisX.exe` legen.

*Profil exportieren* macht dasselbe, fragt aber nach Dateiname und Speicherort.

Ist kein gültiger Skriptordner eingestellt, sucht Axis X eine Profil-Datei zuerst in
`%LOCALAPPDATA%\AxisX\Profile`, dann in `Profile` neben der `AxisX.exe`, und lädt sie automatisch. Die
Datei enthält nur, was Axis zum Anzeigen braucht (Name, Defname, Grafik, Farbe, Kategorie). Die
Ausrüstungsvorschau der NPCs braucht die vollen Skripte. Siehe `LIESMICH.txt` im Profil-Ordner.

**Registry-Werte** (`HKCU\Software\Sphere\GM Tools`, alle werden in der App gesetzt)

| Wert | Bedeutung |
|---|---|
| `Language` | `eng` (Standard) oder `deu` |
| `Theme` | `dark` (Standard) oder `light` |
| `DisableToolbar` | 1 = keine Mini-Leiste, normal in die Taskleiste minimieren |
| `MiniBar X`, `MiniBar Y` | Position der Mini-Leiste |
| `Window Width`, `Window Height`, `Window Maximized` | Fenstergröße der letzten Sitzung |
| `DashQuickActions`, `RecentAdds` | Schnellaktionen und zuletzt erzeugte Items der Übersicht |
| `Staff Prefix` | Konto-Präfix, das in der Begrüßung weggelassen wird (Standard `+staff_`) |
| `CheckUpdates` | 0 = keine Update-Prüfung beim Start (Standard 1) |
| `LastUpdateCheck` | Datum der letzten Update-Prüfung (`JJJJMMTT`) |

**Eigenes Logo und Icon**
- *Einstellungen -> Allgemein -> "Logo (Datei oder URL)"*: ein Bild (PNG/BMP/JPG/GIF/ICO) für den Kopf
  der Seitenleiste.
- *Einstellungen -> Allgemein -> "Icon (.ico, Datei oder URL)"*: ersetzt das Fenster- und
  Taskleisten-Icon.
- Beides nimmt einen lokalen Pfad oder eine `http(s)://`-URL. URLs werden in
  `%LOCALAPPDATA%\AxisX\Branding` zwischengespeichert.

**Remote-Konsole**
- Aufruf über die Übersicht (*Verbinden*) oder über *GM-Befehle -> Remote-Konsole*.
- Ein Login-Dialog fragt Server-IP, Port, Konto und Passwort eines Kontos mit ausreichendem PLevel ab.
  Gesendet wird erst, wenn man mit OK bestätigt.
- Nutzt der Shard ein Staff-Präfix (z. B. `+staff_name`), wird es über den Registry-Wert
  `Staff Prefix` eingestellt.

**Sprache**
- Die Sprache wird unter *Einstellungen -> Allgemein -> Sprache* gewählt, oder über den Registry-Wert
  `Language` (`eng` oder `deu`). Sie gilt ab dem nächsten Start.
- Dateien:
  - `Language\<code>.lng`: Meldungstexte.
  - `Language\eng.ui.txt`: Übersetzungen der Oberflächentexte, eine Zeile je Text in der Form
    `Deutsch<TAB>Englisch`.
- `tools\Extract-Translatable.ps1 -Table Release\Language\eng.ui.txt` listet Texte auf, die in der
  Tabelle noch fehlen.

### Entwickler-Werkzeuge (`tools\`)

| Skript | Zweck |
|---|---|
| `Build-Axis.ps1 [-Install -CloseRunning -Start]` | mit MSBuild bauen, nach `Release\` kopieren, starten |
| `Release-Axis.ps1 -Version 1.1 [-Makensis <Pfad>] [-Publish]` | Version setzen, Exe und Installer bauen; mit `-Publish` committen, taggen und das GitHub-Release anlegen (Text aus `release\changelog.txt`) |
| `Capture-AxisWindow.ps1` | Bildschirmfoto des Axis-Fensters oder eines Popups, optional nach einem Klick |
| `Check-AxisLabels.ps1 [-Width -Height / -Maximize]` | meldet Beschriftungen auf allen Seiten, deren Text nicht passt |
| `Check-AxisPopups.ps1` | öffnet die Popup-Dialoge nacheinander, prüft sie und schließt sie wieder |
| `Test-AxisMiniBar.ps1` | minimiert Axis und klickt Symbole der Mini-Leiste an |
| `Make-AxisIcons.ps1 [-App]` | erzeugt Installer-Icons und Kopfbild (optional auch das Programm-Icon) |
| `Extract-Translatable.ps1`, `Wrap-Translatable.ps1` | Übersetzungstabelle pflegen |

Die Prüfskripte schicken nur Fensternachrichten an Axis selbst (keine Tastendrücke an andere Fenster).

### Zukunftsprojekte

Noch nicht geplant - wird aufgegriffen, sobald Bedarf besteht:

- **macOS und Linux.** Axis X baut auf MFC und Win32 auf und lässt sich nicht direkt portieren. Die eigentliche
  Hürde ist die Befehlsübergabe ins Spiel: Axis tippt Befehle ins Client-Fenster, und das funktioniert unter
  macOS/Linux anders (nur X11, unter Wayland eingeschränkt, macOS braucht die Freigabe unter
  "Bedienungshilfen"). Der vorgesehene Weg wäre ein lokaler Befehlskanal im Spiel-Client (auf ClassicUO-Basis)
  plus eine plattformübergreifende Axis in C#/.NET, die die Dateileser von ClassicUO mitnutzen könnte. Axis
  unter Wine hilft nur, wenn auch der Client unter Wine läuft.

### Lizenz

GPL v2 - siehe [LICENSE](LICENSE). Basiert auf Axis (Philip A. Esterle/Adron) und Axis2 (Benoit Croussette).
