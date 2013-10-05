# Goat3D Blender >2.5 exporter
import bpy;
from bpy_extras.io_utils import ExportHelper

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
	bl_label = "Goat3D scene export"

	fname = bpy.props.Stringproperty(subtype="FILE_PATH")

	@classmethod
	def poll(cls, ctx):
		return ctx.object is not None

	def execute(self, context):
		file = open(self.filepath, "w")
		file.write("foobar " + ctx.object.name)
		return {'FINISHED'}

def menu_func(self, ctx):
	self.layout.operator_context = 'INVOKE_DEFAULT'
	self.layout.operator(ExportGoat3D.bl_idname, text="Goat3D scene export")

def register():
	bpy.utils.register_module(__name__)
	bpy.types.INFO_MT_file_export.append(menu_func)

def unregister():
	bpy.utils.unregister_module(__name__)
	bpy.types.INFO_MT_file_export.remove(menu_func)

if __name__ == "__main__":
	register()
