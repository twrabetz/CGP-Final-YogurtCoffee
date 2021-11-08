.PHONY : all

#n.b. the '-y' sets autoexec scripts to 'on' so that driver expressions will work
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	BLENDER = /Applications/Blender.app/Contents/MacOS/Blender -y
else ifeq ($(UNAME_S),Linux)
	BLENDER = ../../blender-2.93.4-linux-x64/blender -y
else
	BLENDER = blender
endif

EXPORT_MESHES=export-meshes.py
EXPORT_WALKMESHES=export-walkmeshes.py
EXPORT_SCENE=export-scene.py

DIST=../dist

all : \
	$(DIST)/castle.pnct \
	$(DIST)/castle.scene \


$(DIST)/castle.scene : castle.blend $(EXPORT_SCENE)
	$(BLENDER) --background --python $(EXPORT_SCENE) -- '$<':Collection '$@'

$(DIST)/castle.pnct : castle.blend $(EXPORT_MESHES)
	$(BLENDER) --background --python $(EXPORT_MESHES) -- '$<':Collection '$@'
