FROM emscripten/emsdk:latest
WORKDIR /app
COPY . .

RUN emsdk install latest
RUN emsdk activate latest

RUN rm -rf CMakeCache.txt CMakeFiles/

RUN emcmake cmake .
RUN emmake make

RUN emcc src/main.cpp \
    src/framework/geometry.cpp \
    src/framework/gpuProgram.cpp \
    src/framework/shader.cpp \
    src/framework/texture.cpp \
    src/non-euclidean/curvature.cpp \
    src/non-euclidean/geomCamera.cpp \
    -I./src \
    -I./src/framework \
    -I./src/non-euclidean \
    -o real-time-rendering-in-curved-spaces.html \
    --preload-file src/shaders \
    -s USE_WEBGL2=1

EXPOSE 8080
CMD ["emrun", "--no_browser", "--port", "8080", "real-time-rendering-in-curved-spaces.html"]