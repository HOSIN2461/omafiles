# Omafiles — Magyar fájlkezelő

Qt Quick és GIO alapú fájlkezelő Linuxra. Gyors, könnyű, testreszabható.

## Jellemzők

- Lista és rács nézetek, fülek, osztott nézet (F3), fa kibontás
- Gépelés közbeni keresés, testreszabható oszlopok
- Másolás/kivágás/beillesztés (rendszer vágólap), áthelyezés, átnevezés
- Csoportos átnevezés, kukába helyezés, törlés — visszavonással
- Miniatűrok (képek, videó, PDF) a freedesktop spec szerint
- Oldalsáv: eszközök csatolás/kicsatolás, Kuka, Legutóbbi, Kedvencek, könyvjelzők
- Keresés: rekurzív fájlnév + teljes szöveges (via `localsearch`)
- Tömörítés/kibontás (zip, tar.xz, 7z, titkosított zip)
- Többablakos egypéldányos, `org.freedesktop.FileManager1` D-Bus integráció
- Egyéni helyi menü műveletek egyszerű TOML fájlokból

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

## Követelmények

Arch Linux vagy bármely rolling-release disztribúció Qt6-tal.
Függőségek: `qt6-base`, `qt6-declarative`, `glib2`, `gvfs`, `libarchive`, `tinysparql`.
Opcionális: `gvfs-smb`/`gvfs-mtp`/`gvfs-gphoto2` hálózati megosztásokhoz,
`ffmpegthumbnailer` videó miniatűrökhöz, `localsearch` teljes szöveges kereséshez.

## Licenc

MIT
