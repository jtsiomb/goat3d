bl_info = {
	"name": "Goat3D scene",
	"author": "John Tsiombikas",
	"version": (0, 1),
	"blender": (2, 63, 0),
	"location": "File > Import-Export",
	"description": "Mutant Stargoat, Goat3D scene file format: http://code.google.com/p/goat3d/",
	"category": "Import-Export"
}

if "bpy" in locals():
	import imp
	if "export_goat" in locals():
		imp.reload(export_goat)

import bpy
from bpy_extras.io_utils import ExportHelper

class ExportGoat3D(bpy.types.Operator, ExportHelper):
	bl_idname = "export_scene.goat3d"
	bl_label = "Export Goat3D Scene"
	bl_options = {'PRESET'}
	filename_ext = ".goatsce"
	filter_glob = bpy.props.StringProperty(default="*.goatsce", options={'HIDDEN'})

	def execute(self, ctx):
		from . import export_goat
		if not export_goat.export(self, ctx, self.filepath):
			return {'CANCELLED'}
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
