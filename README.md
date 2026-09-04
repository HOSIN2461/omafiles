# Omafiles — Magyar fájlkezelő Omarchy-hoz

Az [Omarchy](https://omarchy.org) natív fájlkezelője, Qt Quick és GIO alapon.
A GNOME Files (Nautilus) helyettesítője — ugyanazok a billentyűparancsok,
indítási szemantika és D-Bus integráció, az Omarchy témájával.

> **Tesztelési előnézet.** Az omafiles a meglévő fájlkezelő MELLETT települ,
> és semmit sem módosít, amíg nem váltasz. A mellékelt váltó eszköz
> segítségével flippingelhetsz a kettő között, és visszaállítja az eredeti
> állapotot bájtpontosan.

## Amit kapsz

- Lista és rács nézetek, fülek, osztott nézet (F3), fa kibontás,
  sávok + Ctrl+L, gépelés közbeni keresés, testreszabható oszlopok
- Minden írási művelet — másolás/kivágás/beillesztés (rendszer vágólap),
  áthelyezés, átnevezés, csoportos átnevezés, kukába helyezés, törlés —
  visszavonással/szétvonással és haladás jelzővel
- Miniatűrok (képek, videó, PDF) a freedesktop spec szerint
- Oldalsáv: eszközök csatolás/kicsatolás/kiadás hálózattal (`smb://`, `sftp://`),
  Kuka, Legutóbbi, Kedvencek, könyvjelzők
- Keresés: rekurzív fájlnév + teljes szöveges (via `localsearch`),
  dátum és típus szűrők
- Tömörítés/kibontás (zip, tar.xz, 7z, titkosított zip)
- Omarchy téma végig — követi az aktív élő témát
- Egyéni helyi menü műveletek egyszerű TOML fájlokból
- Többablakos egypéldányos, `org.freedesktop.FileManager1`

## Telepítés

Töltsd le a csomagot a [legújabb kiadásból](https://github.com/HOSIN2461/omafiles/releases)
és telepítsd:

```bash
# Arch Linux
makepkg -si
```

Vagy építsd magad:

```bash
git clone https://github.com/HOSIN2461/omafiles.git
cd omafiles/packaging
makepkg -si
```

## Váltás

```bash
omafiles-switch omafiles     # omafiles legyen az alapértelmezett
omafiles-switch nautilus     # vissza a Nautilus-hoz
omafiles-switch toggle       # váltás
omafiles-switch status       # mi az alapértelmezett
```

Vagy az asztalról: futtasd az `omafiles-switch install-menu` parancsot egyszer,
és az Omarchy Toggle menü (`SUPER+CTRL+O`) megjeleníti az **Omafiles Fájlkezelő**
sort — válaszd a váltáshoz.

## Eltávolítás

```bash
omafiles-switch nautilus && omafiles-switch remove-menu
sudo pacman -R omafiles
``"

## Követelmények

Arch Linux Omarchy-val. A függőségek (`qt6-base`, `qt6-declarative`, `glib2`,
`gvfs`, `libarchive`, `tinysparql`) az Omarchy alap telepítésében szerepelnek.
Opcionális: `gvfs-smb`/`gvfs-mtp`/`gvfs-gphoto2` hálózati megosztásokhoz,
`ffmpegthumbnailer` videó miniatűrökhöz, `localsearch` teljes szöveges kereséshez.

## Fejlesztés

```bash
./bin/build      # cmake + ninja a build/ mappába, majd futtatás
./bin/test       # fej nélküli tesztek (ctest)
./bin/install    # felhasználói telepítés: ~/.local/bin symlink
```

## Licenc

MIT
