/*
goat3d - 3D scene, and animation file format library.
Copyright (C) 2013-2023  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdlib.h>
#include <ctype.h>
#include "goat3d.h"
#include "g3dscn.h"
#include "log.h"
#include "json.h"

int g3dimpl_loadgltf(struct goat3d *g, struct goat3d_io *io)
{
	long i, filesz;
	char *filebuf;
	struct json_obj root;
	struct json_value *jval;

	if(!(filebuf = malloc(4096))) {
		goat3d_logmsg(LOG_ERROR, "goat3d_load: failed to allocate file buffer\n");
		return -1;
	}
	filesz = io->read(filebuf, 4096, io->cls);
	if(filesz < 2) {
		free(filebuf);
		return -1;
	}
	for(i=0; i<filesz; i++) {
		if(!isspace(filebuf[i])) {
			if(filebuf[i] != '{') {
				free(filebuf);
				return -1;		/* not json */
			}
			break;
		}
	}
	free(filebuf);

	/* alright, it looks like json, load into memory and parse it to continue */
	filesz = io->seek(0, SEEK_END, io->cls);
	io->seek(0, SEEK_SET, io->cls);
	if(!(filebuf = malloc(filesz + 1))) {
		goat3d_logmsg(LOG_ERROR, "goat3d_load: failed to load file into memory\n");
		return -1;
	}
	if(io->read(filebuf, filesz, io->cls) != filesz) {
		goat3d_logmsg(LOG_ERROR, "goat3d_load: EOF while reading file\n");
		free(filebuf);
		return -1;
	}
	filebuf[filesz] = 0;

	json_init_obj(&root);
	if(json_parse(&root, filebuf) == -1) {
		free(filebuf);
		return -1;
	}
	free(filebuf);

	/* a valid gltf file needs to have an "asset" node with a version number */
	if(!(jval = json_lookup(&root, "asset.version"))) {
		json_destroy_obj(&root);
		return -1;
	}

	/* ... */

	json_destroy_obj(&root);
	return 0;
}
