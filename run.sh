#!/bin/bash

qemu-system-x86_64 -s -S -drive format=raw,file=build/image.iso
