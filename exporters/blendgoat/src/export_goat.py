# Goat3D Blender >2.63 exporter
import bpy
import ctypes
from ctypes import *
from ctypes.util import find_library

GOAT3D_MAT_ATTR_DIFFUSE			= "diffuse".encode('utf-8')
GOAT3D_MAT_ATTR_SPECULAR		= "specular".encode('utf-8')
GOAT3D_MAT_ATTR_SHININESS		= "shininess".encode('utf-8')
GOAT3D_MAT_ATTR_NORMAL			= "normal".encode('utf-8')
GOAT3D_MAT_ATTR_BUMP			= "bump".encode('utf-8')
GOAT3D_MAT_ATTR_REFLECTION		= "reflection".encode('utf-8')
GOAT3D_MAT_ATTR_TRANSMISSION	= "transmission".encode('utf-8')
GOAT3D_MAT_ATTR_IOR				= "ior".encode('utf-8')
GOAT3D_MAT_ATTR_ALPHA			= "alpha".encode('utf-8')

# these must match the goat3d_vertex_attr enumeration values in goat3d.h
GOAT3D_MESH_ATTR_VERTEX			= 0
GOAT3D_MESH_ATTR_NORMAL			= 1
GOAT3D_MESH_ATTR_TANGENT		= 2
GOAT3D_MESH_ATTR_TEXCOORD		= 3
GOAT3D_MESH_ATTR_SKIN_WEIGHT	= 4
GOAT3D_MESH_ATTR_SKIN_MATRIX	= 5
GOAT3D_MESH_ATTR_COLOR			= 6


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
	export_meshes(ctx, goat, lib)
	export_nodes(ctx, goat, lib)

	lib.goat3d_save(goat, fname.encode('utf-8'))
	lib.goat3d_free(goat)
	return True

def export_env(ctx, goat, lib):
	return False

def export_material(bmtl, goat, lib):
	name = bmtl.name.encode("utf-8")

	mtl = lib.goat3d_create_mtl()
	lib.goat3d_set_mtl_name(mtl, name)

	s = bmtl.diffuse_intensity
	col = bmtl.diffuse_color
	lib.goat3d_set_mtl_attrib4f(mtl, GOAT3D_MAT_ATTR_DIFFUSE, c_float(col[0] * s), c_float(col[1] * s), c_float(col[2] * s), 1.0)

	s = bmtl.specular_intensity
	col = bmtl.specular_color
	lib.goat3d_set_mtl_attrib4f(mtl, GOAT3D_MAT_ATTR_SPECULAR, c_float(col[0] * s), c_float(col[1] * s), c_float(col[2] * s), 1.0)
	lib.goat3d_set_mtl_attrib1f(mtl, GOAT3D_MAT_ATTR_SHININESS, c_float(bmtl.specular_hardness))
	lib.goat3d_set_mtl_attrib1f(mtl, GOAT3D_MAT_ATTR_ALPHA, c_float(bmtl.alpha))

	if bmtl.raytrace_mirror.use:
		lib.goat3d_set_mtl_attrib1f(mtl, GOAT3D_MAT_ATTR_REFLECTION, c_float(bmtl.raytrace_mirror.reflect_factor))
	if bmtl.use_transparency and bmtl.transparency_method == 'RAYTRACE':
		lib.goat3d_set_mtl_attrib1f(mtl, GOAT3D_MAT_ATTR_IOR, c_float(bmtl.raytrace_transparency.ior))
		lib.goat3d_set_mtl_attrib1f(mtl, GOAT3D_MAT_ATTR_TRANSMISSION, 1.0)

	# grab all the textures and apply them to the correct attributes
	for texslot in bmtl.texture_slots:
		if not texslot:
			continue

		tex = texslot.texture
		if tex.type != 'IMAGE':
			print("ignoring texture " + tex.name + ": not an image!")
			continue

		fname = tex.image.filepath.encode('ascii')

		attr = ""
		if texslot.use_map_color_diffuse or texslot.use_map_diffuse:
			attr = GOAT3D_MAT_ATTR_DIFFUSE
		elif texslot.use_map_color_spec or texslot.use_map_specular:
			attr = GOAT3D_MAT_ATTR_SPECULAR
		elif texslot.use_map_color_reflection or texslot.use_map_reflect:
			attr = GOAT3D_MAT_ATTR_REFLECTION
		elif texslot.use_map_hardness:
			attr = GOAT3D_MAT_ATTR_SHININESS
		elif texslot.use_map_alpha:
			attr = GOAT3D_MAT_ATTR_ALPHA
		elif texslot.use_map_normal:
			attr = GOAT3D_MAT_ATTR_NORMAL

		if attr != "":
			lib.goat3d_set_mtl_attrib_map(mtl, attr, fname)

	lib.goat3d_add_mtl(goat, mtl)
	return mtl


def export_meshes(ctx, goat, lib):
	print("exporting " + str(len(ctx.scene.objects)) + " objects")
	for obj in ctx.scene.objects:
		if obj.type != 'MESH':
			continue

		bmesh = obj.data
		# make sure we get a tesselated mesh
		bmesh.update(calc_tessface = True)

		# create goat3d mesh and set the data
		mesh = lib.goat3d_create_mesh()
		lib.goat3d_set_mesh_name(mesh, bmesh.name.encode("utf-8"))

		# get the material, add it to the scene and apply it to this mesh
		for bmtl in bmesh.materials:
			mtl = export_material(bmtl, goat, lib)
			lib.goat3d_set_mesh_mtl(mesh, mtl)
			break	# we only care about one material

		for vert in bmesh.vertices:
			v = vert.co
			n = vert.normal
			lib.goat3d_add_mesh_attrib3f(mesh, GOAT3D_MESH_ATTR_VERTEX, v[0], v[1], v[2])
			lib.goat3d_add_mesh_attrib3f(mesh, GOAT3D_MESH_ATTR_NORMAL, n[0], n[1], n[2])

		for face in bmesh.tessfaces:
			fverts = face.vertices
			lib.goat3d_add_mesh_face(mesh, fverts[0], fverts[1], fverts[2])

		lib.goat3d_add_mesh(goat, mesh)
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

	lib.goat3d_set_mtl_attrib1f.argtypes = [c_void_p, c_char_p, c_float]
	lib.goat3d_set_mtl_attrib1f.restype = None

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

	lib.goat3d_add_mesh_attrib3f.argtypes = [c_void_p, c_int, c_float, c_float, c_float]
	lib.goat3d_add_mesh_attrib3f.restype = None

	lib.goat3d_add_mesh_attrib4f.argtypes = [c_void_p, c_int, c_float, c_float, c_float, c_float]
	lib.goat3d_add_mesh_attrib4f.restype = None

	lib.goat3d_add_mesh_face.argtypes = [c_void_p, c_int, c_int, c_int]
	lib.goat3d_add_mesh_face.restype = None

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
