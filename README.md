# Spennydl's demo renderer

This is a simple renderer built to be a testbed for further explorations
into rendering techniques. It's built to be dirt simple, not too abstract, 
and flexible. It implements a fairly standard PBR pipeline as a baseline.

## Run the Baseline Demo

The baseline demo renders a model, and that's all.

To build and run, from the root of the repository:

```sh
$ cmake -S . -B build
$ cmake --build build -- -j 8
$ ./build/baseline/baseline
```

## Resource Credits

The fox model in the baseline demo was made by Tibo and retrieved from [here.](https://gtibo.itch.io/hooded-fox)

The cloudy sky HDR skybox was from HDRI-Hub and retrieved from [here.](https://www.hdri-hub.com/hdrishop/freesamples/freehdri/item/76-hdr-sky-cloudy)
