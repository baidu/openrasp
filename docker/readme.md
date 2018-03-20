## OpenRASP docker images

To build docker image with a specific version of OpenRASP agent, use the `--build-arg` arguments, e.g

```
docker build . --build-arg version_testcase=v1.0.1 --build-arg version_rasp=v0.24
```

