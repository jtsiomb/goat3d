# Goat3D Blender >2.5 exporter
import bpy;
from bpy_extras.io_utils import ExportHelper
import ctypes
from ctypes import *
from ctypes.util import find_library
import os

bl_info = {
	"name": "Goat3D scene",
	"author": "John Tsiombikas",
	"version": (0, 1),
	"location": "File > Import-Export",
	"description": "Mutant Stargoat, Goat3D scene file format: http://code.google.com/p/goat3d/",
	"category": "Import-Export"
}

class ExportGoat3D(bpy.types.Operator, ExportHelper):
	bl_idname = "export_scene.goat3d"
	bl_label = "Export Goat3D Scene"
	bl_options = {'PRESET'}
	filename_ext = ".goatsce"
	filter_glob = bpy.props.StringProperty(default="*.goatsce", options={'HIDDEN'})

	@classmethod
	def poll(cls, ctx):
		return ctx.object is not None

	def execute(self, ctx):
		libname = find_library("goat3d")
		if not libname:
			self.report({'ERROR'}, "Could not find the goat3d library! make sure it's installed.")
			return {'CANCELLED'}

		libgoat = CDLL(libname)
		if not libgoat:
			self.report({'ERROR'}, "Could not open goat3d library!")
			return {'CANCELLED'}

		goat3d_create = libgoat.goat3d_create
		goat3d_create.argtypes = None
		goat3d_create.restype = c_void_p

		goat3d_free = libgoat.goat3d_free
		goat3d_free.argtypes = [c_void_p]
		goat3d_free.restype = None

		goat3d_save = libgoat.goat3d_save
		goat3d_save.argtypes = [c_void_p, c_char_p]

		print("Exporting goat3d file: " + self.filepath)

		goat = goat3d_create()
		if not goat:
			self.report({'ERROR'}, "Failed to create goat3d object")
			return {'CANCELLED'}

		print(type(goat))
		print(type(self.filepath))
		goat3d_save(goat, c_char_p(self.filepath))
		goat3d_free(goat)
		return {'FINISHED'}

def menu_func(self, ctx):
	self.layout.operator_context = 'INVOKE_DEFAULT'
	self.layout.operator(ExportGoat3D.bl_idname, text="Goat3D scene export (.goatsce)")

def register():
	bpy.utils.register_module(__name__)
	bpy.types.INFO_MT_file_export.append(menu_func)

def unregister():
	bpy.utils.unregister_module(__name__)
	bpy.types.INFO_MT_file_export.remove(menu_func)

if __name__ == "__main__":
	register()
