#!/bin/bash
cmake -B build -DBUILD_TEST='OFF' && cmake --build build -j8 && prime-run ./myprogram