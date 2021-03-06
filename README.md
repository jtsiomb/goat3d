goat3d v2
=========

About
-----
Goat3D is a hierarchical 3D scene, character, and animation file format set,
targeting mostly real-time applications. The goat3d specification defines 2
complimentary file formats:
 - The main goat3d file format defines meshes, lights, cameras, environmental
   parameters, materials, and transformation hierarchies.
 - The animation file format defines a sequence of keyframes and interpolation
   parameters, for any number of nodes, defined in the main scene file.

The specification defines a hierarchical structure (see `doc/goatfmt`, and
`doc/goatanimfmt` for details), which can be stored in either text or binary
form. An application using the provided library to read/write goat3d files,
should be able to handle either variant, with no extra effort.

This project provides the specification of the file formats, a simple library
with a clean C API for reading and writing files in the goat3d scene and
animation file formats, as well as a number of tools dealing with such files.

Specifically, at the moment, the goat3d project provides the following:
 - *libgoat3d*, a library for reading and writing goat3d scene and animation files.
 - *ass2goat*, a universal 3D asset conversion utility based on the excellent
   assimp library, from a huge number of 3D file formats to the goat3d file
   format.
 - *goatview*, a 3D scene and animation preview tool, based on OpenGL.
 - *goatprim*, a procedural 3D model (primitive) generator for quick testing.

Changes in v2
-------------
An animation (`ANIM` node) can be in a separate file like the original goat3d
specification, or as a child of `SCENE`, in the same file as the main scene
description.

The rest of the changes in goat3d v2 are strictly in the implementation, and do
not affect user programs at all. All the code has been re-written in C, and
`libtreestore` is used to read/write text and binary hierarchical files
following the structure defined by goat3d.

A number of unmaintained tools have been dropped (for now), opting to support
the `ass2goat` assimp converter pipeline, instead of keeping a bunch of
half-baked exporters for various programs that used to support a subset of the
goat3d features. The blender exporter might return at some point, but I have no
intention to maintain exporters for any other programs at this stage.
Contributions are certainly welcome, as well as anyone who wishes to write and
maintain some other specific tool/exporter.

Another change is that visual studio project files for windows have been
dropped. The supported way to build windows binaries is with mingw either on
windows, or cross-compiled from a UNIX system. If someone wishes to contribute
and maintain files for alternative build systems, I'm willing to include them,
as long as they can reside in a separate directory, like `contrib/vcbuild`.

License
-------
Copyright (C) 2014-2018 John Tsiombikas <nuclear@member.fsf.org>

Goat3D is free software, you may use, modify and/or redistribute it under the
terms of the GNU Lesser General Public License v3, or at your option any later
version published by the Free Software Foundation. See COPYING and
COPYING.LESSER for details.

Build
-----
To build and install libgoat3d on UNIX, run the usual:

    ./configure
    make
    make install

See `./configure --help` for a complete list of build-time options.

To cross-compile for windows with mingw-w64, try the following incantation:

    ./configure --prefix=/usr/i686-w64-mingw32
    make CC=i686-w64-mingw32-gcc AR=i686-w64-mingw32-ar sys=mingw
    make install sys=mingw

The rest of the tools can be built and installed in the exact same way from
their respective subdirectories.
