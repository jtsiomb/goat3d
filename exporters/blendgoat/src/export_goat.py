# Goat3D Blender >2.5 exporter
import bpy;
import ctypes
from ctypes import *
from ctypes.util import find_library


def export(oper, ctx, fname):
	print("Exporting goat3d file: " + fname)

	# load all relevant functions from libgoat3d
	libname = find_library("goat3d")
	if not libname:
		oper.report({'ERROR'}, "Could not find the goat3d library! make sure it's installed.")
		return False

	libgoat = CDLL(libname)
	if not libgoat:
		oper.report({'ERROR'}, "Could not open goat3d library!")
		return False

	goat3d_create = libgoat.goat3d_create
	goat3d_create.argtypes = None
	goat3d_create.restype = c_void_p

	goat3d_free = libgoat.goat3d_free
	goat3d_free.argtypes = [c_void_p]
	goat3d_free.restype = None

	goat3d_set_name = libgoat.goat3d_set_name
	goat3d_set_name.argtypes = [c_void_p, c_char_p]

	goat3d_set_ambient3f = libgoat.goat3d_set_ambient3f
	goat3d_set_ambient3f.argtypes = [c_void_p, c_float, c_float, c_float]
	goat3d_set_ambient3f.restype = None

	goat3d_add_mtl = libgoat.goat3d_add_mtl
	goat3d_add_mtl.argtypes = [c_void_p, c_void_p]
	goat3d_add_mtl.restype = None

	goat3d_create_mtl = libgoat.goat3d_create_mtl
	goat3d_create_mtl.argtypes = None
	goat3d_create_mtl.restype = c_void_p

	goat3d_set_mtl_name = libgoat.goat3d_set_mtl_name
	goat3d_set_mtl_name.argtypes = [c_void_p, c_char_p]
	goat3d_set_mtl_name.restype = None

	goat3d_set_mtl_attrib4f = libgoat.goat3d_set_mtl_attrib4f
	goat3d_set_mtl_attrib4f.argtypes = [c_void_p, c_char_p, c_float, c_float, c_float, c_float]
	goat3d_set_mtl_attrib4f.restype = None

	goat3d_set_mtl_attrib_map = libgoat.goat3d_set_mtl_attrib_map
	goat3d_set_mtl_attrib_map.argtypes = [c_void_p, c_char_p, c_char_p]
	goat3d_set_mtl_attrib_map.restype = None

	goat3d_add_mesh = libgoat.goat3d_add_mesh
	goat3d_add_mesh.argtypes = [c_void_p, c_void_p]
	goat3d_add_mesh.restype = None

	goat3d_create_mesh = libgoat.goat3d_create_mesh
	goat3d_create_mesh.argtypes = None
	goat3d_create_mesh.restype = c_void_p

	goat3d_set_mesh_name = libgoat.goat3d_set_mesh_name
	goat3d_set_mesh_name.argtypes = [c_void_p, c_char_p]
	goat3d_set_mesh_name.restype = None

	goat3d_set_mesh_mtl = libgoat.goat3d_set_mesh_mtl
	goat3d_set_mesh_mtl.argtypes = [c_void_p, c_void_p]
	goat3d_set_mesh_mtl.restype = None

	goat3d_set_mesh_attribs = libgoat.goat3d_set_mesh_attribs
	goat3d_set_mesh_attribs.argtypes = [c_void_p, c_int, c_void_p, c_int]
	goat3d_set_mesh_attribs.restype = None

	goat3d_set_mesh_faces = libgoat.goat3d_set_mesh_faces
	goat3d_set_mesh_faces.argtypes = [c_void_p, c_void_p, c_int]
	goat3d_set_mesh_faces.restype = None

	goat3d_add_node = libgoat.goat3d_add_node
	goat3d_add_node.argtypes = [c_void_p, c_void_p]
	goat3d_add_node.restype = None

	goat3d_create_node = libgoat.goat3d_create_node
	goat3d_create_node.argtypes = None
	goat3d_create_node.restype = c_void_p

	goat3d_set_node_name = libgoat.goat3d_set_node_name
	goat3d_set_node_name.argtypes = [c_void_p, c_char_p]
	goat3d_set_node_name.restype = None

	goat3d_set_node_object = libgoat.goat3d_set_node_object
	goat3d_set_node_object.argtypes = [c_void_p, c_int, c_void_p]
	goat3d_set_node_object.restype = None

	goat3d_add_node_child = libgoat.goat3d_add_node_child
	goat3d_add_node_child.argtypes = [c_void_p, c_void_p]
	goat3d_add_node_child.restype = None

	goat3d_set_node_position = libgoat.goat3d_set_node_position
	goat3d_set_node_position.argtypes = [c_void_p, c_float, c_float, c_float, c_long]
	goat3d_set_node_position.restype = None

	goat3d_set_node_rotation = libgoat.goat3d_set_node_rotation
	goat3d_set_node_rotation.argtypes = [c_void_p, c_float, c_float, c_float, c_float, c_long]
	goat3d_set_node_rotation.restype = None

	goat3d_set_node_scaling = libgoat.goat3d_set_node_scaling
	goat3d_set_node_scaling.argtypes = [c_void_p, c_float, c_float, c_float, c_long]
	goat3d_set_node_scaling.restype = None

	goat3d_set_node_pivot = libgoat.goat3d_set_node_pivot
	goat3d_set_node_pivot.argtypes = [c_void_p, c_float, c_float, c_float]
	goat3d_set_node_pivot.restype = None

	goat3d_save = libgoat.goat3d_save
	goat3d_save.argtypes = [c_void_p, c_char_p]

	# initiaize a goat3d object and start filling it up with data
	goat = goat3d_create()
	if not goat:
		oper.report({'ERROR'}, "Failed to create goat3d object")
		return False

	goat3d_save(goat, fname.encode('utf-8'))
	goat3d_free(goat)
	return True


