# 1. Introduction

Flutter embedder can run Linux (ARM based) embedded system with wayland support. This applcation does NOT need GTK or X system.
This project is inspired by flutter wayland project (https://github.com/chinmaygarde/flutter_wayland).

<img src="assets/logo.png" alt="logo" style="zoom: 25%;" />





# 2. Build Steps

### 2.1 Setup env

To start build, a few envrionment variables have to be properly set. For example (We use MyARM toolchain as an example)

```bash
# Flutter workspace
export FLUTTER_WS=`pwd`

## Used to setup cross compiler
export MYARM_TOOLCHAIN=${FLUTTER_WS}/host

## Used to include fastjson under '${FLUTTER_ENGINE_DIR}/third_party/rapidjson/include/'
export FLUTTER_ENGINE_DIR=${FLUTTER_WS}/engine

## Used to include APIs of flutter
export FLUTTER_ENGINE_ROOT=${FLUTTER_WS}/engine_root  # Engine built result
```



Here is an example directory structure of 'FLUTTER_ENGINE_ROOT'

```bash
$ tree -L 2
.
├── include
│   ├── flutter_embedder.h
│   └── X11
└── lib
    └── libflutter_engine.so
```

Here X11 is needed by some header files for compilation ONLY. If your compilation does NOT need X11 headers, you can ignore it at all.



### 2.2 Build

```bash
$ mkdir build; cd build
$ cmake ..

$ make
```

The final binary is under '**flutter_embeder**'.





# 3. Example

### 3.1 build a flutter example and upload using adb

```bash
$ flutter build bundle

## Then use adb or any other tools to push lib and bundle to device, for example
$ adb push build/flutter_assets /your/desired/directory
## Flutter embedder
$ adb push flutter_embeder /your/desired/directory
```


### 3.2 run

```bash
## Go to /your/desired/directory
./flutter_embeder -d "800x360" flutter_assets/
```



# 4. Contributors

Tuo Lingyun

Wang Wendong

Chen Bingyi

Li Hanfang


# 5. Contact Info

If you have problems when run this, please contact `info@xcvmbyte.com` for further infomation.

