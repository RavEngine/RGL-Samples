# RGL-Samples
Usage examples for [RavEngine Graphics Library](https://github.com/RavEngine/RGL).

### Building
1. Clone recursive to get all submodules:
```sh
git clone https://github.com/RavEngine/RGL-Samples --depth=1 --recurse-submodules
```
2. Execute one of the provided config scripts. You can run multiple scripts if you want. 
```sh
cd config
./init-mac.sh& ./init-ios.sh&
```
3. Open the generated IDE project in `build/` and run the examples

### Running
- To select the underlying graphics API (all except Hello Triangle), set the `RGL_BACKEND` environment variable. Example: `RGL_BACKEND=vulkan`. Your target device must support this API. 
