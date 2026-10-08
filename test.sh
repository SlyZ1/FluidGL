#!/bin/bash
cmake -B build -DBUILD_TEST='ON' && cmake --build build -j8 && ctest --test-dir build --output-on-failure