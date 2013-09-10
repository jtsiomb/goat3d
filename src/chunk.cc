#include "goat3d.h"
#include "chunk.h"

ChunkHeader chunk_header(int id)
{
	ChunkHeader hdr;
	hdr.id = id;
	hdr.size = sizeof hdr;
	return hdr;
}

bool write_chunk_header(const ChunkHeader *hdr, goat3d_io *io)
{
	io->seek(-hdr->size, SEEK_CUR, io->cls);
	if(io->write(hdr, sizeof *hdr, io->cls) < (ssize_t)sizeof *hdr) {
		return false;
	}
	return true;
}

bool read_chunk_header(ChunkHeader *hdr, goat3d_io *io)
{
	if(io->read(hdr, sizeof *hdr, io->cls) < (ssize_t)sizeof *hdr) {
		return false;
	}
	return true;
}

void skip_chunk(const ChunkHeader *hdr, goat3d_io *io)
{
	io->seek(hdr->size - sizeof *hdr, SEEK_CUR, io->cls);
}
