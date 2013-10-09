# Goat3D Blender >2.63 exporter
import bpy;
import ctypes
from ctypes import *
from ctypes.util import find_library


def export(oper, ctx, fname):
	print("Exporting goat3d file: " + fname)

	lib = init_libgoat()
	if not lib:
		return False

	# initiaize a goat3d object and start filling it up with data
	goat = lib.goat3d_create()
	if not goat:
		oper.report({'ERROR'}, "Failed to create goat3d object")
		return False

	lib.goat3d_set_name(goat, fname.encode('utf-8'))

	export_env(ctx, goat, lib)
	export_materials(ctx, goat, lib)
	export_meshes(ctx, goat, lib)
	export_nodes(ctx, goat, lib)

	lib.goat3d_save(goat, fname.encode('utf-8'))
	lib.goat3d_free(goat)
	return True

def export_env(ctx, goat, lib):
	return False

def export_materials(ctx, goat, lib):
	return False

def export_meshes(ctx, goat, lib):
	print("exporting " + str(len(ctx.scene.objects)) + " objects")
	for obj in ctx.scene.objects:
		if obj.type != 'MESH':
			continue

		mesh = obj.data
		# make sure we get a tesselated mesh
		mesh.update(calc_tessface = True)

		triangles = []
		for idx, face in enumerate(mesh.tessfaces):
			fverts = face.vertices
			triangles.append(fverts[0])
			triangles.append(fverts[1])
			triangles.append(fverts[2])

		print("creating native array of " + str(len(triangles)) + " triangles")
		IndexArrayType = c_int * len(triangles)
		indices = IndexArrayType(triangles)
	return False


def export_nodes(ctx, goat, lib):
	return False

def init_libgoat():
	# load all relevant functions from libgoat3d
	libname = find_library("goat3d")
	if not libname:
		oper.report({'ERROR'}, "Could not find the goat3d library! make sure it's installed.")
		return None

	lib = CDLL(libname)
	if not lib:
		oper.report({'ERROR'}, "Could not open goat3d library!")
		return None

	lib.goat3d_create.argtypes = None
	lib.goat3d_create.restype = c_void_p

	lib.goat3d_free.argtypes = [c_void_p]
	lib.goat3d_free.restype = None

	lib.goat3d_set_name.argtypes = [c_void_p, c_char_p]

	lib.goat3d_set_ambient3f.argtypes = [c_void_p, c_float, c_float, c_float]
	lib.goat3d_set_ambient3f.restype = None

	lib.goat3d_add_mtl.argtypes = [c_void_p, c_void_p]
	lib.goat3d_add_mtl.restype = None

	lib.goat3d_create_mtl.argtypes = None
	lib.goat3d_create_mtl.restype = c_void_p

	lib.goat3d_set_mtl_name.argtypes = [c_void_p, c_char_p]
	lib.goat3d_set_mtl_name.restype = None

	lib.goat3d_set_mtl_attrib4f.argtypes = [c_void_p, c_char_p, c_float, c_float, c_float, c_float]
	lib.goat3d_set_mtl_attrib4f.restype = None

	lib.goat3d_set_mtl_attrib_map.argtypes = [c_void_p, c_char_p, c_char_p]
	lib.goat3d_set_mtl_attrib_map.restype = None

	lib.goat3d_add_mesh.argtypes = [c_void_p, c_void_p]
	lib.goat3d_add_mesh.restype = None

	lib.goat3d_create_mesh.argtypes = None
	lib.goat3d_create_mesh.restype = c_void_p

	lib.goat3d_set_mesh_name.argtypes = [c_void_p, c_char_p]
	lib.goat3d_set_mesh_name.restype = None

	lib.goat3d_set_mesh_mtl.argtypes = [c_void_p, c_void_p]
	lib.goat3d_set_mesh_mtl.restype = None

	lib.goat3d_set_mesh_attribs.argtypes = [c_void_p, c_int, c_void_p, c_int]
	lib.goat3d_set_mesh_attribs.restype = None

	lib.goat3d_set_mesh_faces.argtypes = [c_void_p, c_void_p, c_int]
	lib.goat3d_set_mesh_faces.restype = None

	lib.goat3d_add_node.argtypes = [c_void_p, c_void_p]
	lib.goat3d_add_node.restype = None

	lib.goat3d_create_node.argtypes = None
	lib.goat3d_create_node.restype = c_void_p

	lib.goat3d_set_node_name.argtypes = [c_void_p, c_char_p]
	lib.goat3d_set_node_name.restype = None

	lib.goat3d_set_node_object.argtypes = [c_void_p, c_int, c_void_p]
	lib.goat3d_set_node_object.restype = None

	lib.goat3d_add_node_child.argtypes = [c_void_p, c_void_p]
	lib.goat3d_add_node_child.restype = None

	lib.goat3d_set_node_position.argtypes = [c_void_p, c_float, c_float, c_float, c_long]
	lib.goat3d_set_node_position.restype = None

	lib.goat3d_set_node_rotation.argtypes = [c_void_p, c_float, c_float, c_float, c_float, c_long]
	lib.goat3d_set_node_rotation.restype = None

	lib.goat3d_set_node_scaling.argtypes = [c_void_p, c_float, c_float, c_float, c_long]
	lib.goat3d_set_node_scaling.restype = None

	lib.goat3d_set_node_pivot.argtypes = [c_void_p, c_float, c_float, c_float]
	lib.goat3d_set_node_pivot.restype = None

	lib.goat3d_save.argtypes = [c_void_p, c_char_p]
	return lib
