/*
goat3d - 3D scene, character, and animation file format library.
Copyright (C) 2013-2014  John Tsiombikas <nuclear@member.fsf.org>

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
#include "goat3d.h"
#include "chunk.h"

using namespace g3dimpl;

ChunkHeader chunk_header(int id)
{
	ChunkHeader hdr;
	hdr.id = id;
	hdr.size = sizeof hdr;
	return hdr;
}

bool g3dimpl::write_chunk_header(const ChunkHeader *hdr, goat3d_io *io)
{
	io->seek(-(long)hdr->size, SEEK_CUR, io->cls);
	if(io->write(hdr, sizeof *hdr, io->cls) < (long)sizeof *hdr) {
		return false;
	}
	return true;
}

bool g3dimpl::read_chunk_header(ChunkHeader *hdr, goat3d_io *io)
{
	if(io->read(hdr, sizeof *hdr, io->cls) < (long)sizeof *hdr) {
		return false;
	}
	return true;
}

void g3dimpl::skip_chunk(const ChunkHeader *hdr, goat3d_io *io)
{
	io->seek(hdr->size - sizeof *hdr, SEEK_CUR, io->cls);
}
