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
	bl_idname = "export.goat3d"
	bl_label = "export"
	bl_options = {'PRESET'}
	filename_ext = ".goatsce"

	fname = bpy.props.StringProperty(subtype="FILE_PATH")

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

		goat3d_save_file = libgoat.goat3d_save_file
		goat3d_save_file.argtypes = [c_void_p, c_char_p]

		self.report({'INFO'}, "Exporting goat3d file")

		goat = goat3d_create()
		if not goat:
			self.report({'ERROR'}, "Failed to create goat3d object")
			return {'CANCELLED'}

		goat3d_save_file(goat, "/tmp/lala.xml")
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
