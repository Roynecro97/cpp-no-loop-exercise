# cpp-no-loop-exercise

A short C++ exercise meant to introduce some standard algorithms.

## Running the exercise

After making sure you have access to the docker image (either by building it
locally or deploying it to an available repository), use the `./stdex` script
as the main entrypoint.

### Starting the exercise

```sh
./stdex start
```

A template named `stage1.cpp` will appear in the current directory, it contains
instructions and a `main()` function for you to fill.

### Verifying your solution

```sh
./stdex verify STAGE
```

You solution (in `STAGE.cpp`) will be compiled and checked using a few IO-based
tests.

If the solution passes both compilation and the tests, the next stage will be
automatically generated.

## Building the docker

```sh
./build_exercise.sh
```
