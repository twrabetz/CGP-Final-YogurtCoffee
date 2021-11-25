#n.b. the '-y' sets autoexec scripts to 'on' so that driver expressions will work
ifeq ($(OS),Windows_NT)
	BLENDER := "C:\Program Files\Blender Foundation\Blender 2.93\blender.exe"
else
	UNAME_S = $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		BLENDER := /Applications/Blender/blender.app/Contents/MacOS/blender -y
	else ifeq ($(UNAME_S),Linux)
		BLENDER := ../../blender-2.93.4-linux-x64/blender -y
	else
		BLENDER := blender
	endif
endif

EXPORT_MESHES=export-meshes.py
EXPORT_WALKMESHES=export-walkmeshes.py
EXPORT_SCENE=export-scene.py

DIST=../dist

.PHONY : all

all : \
	$(DIST)/SquidgeBall.pnct \
	$(DIST)/SquidgeBall.scene \


$(DIST)/SquidgeBall.scene : SquidgeBall_chen.blend $(EXPORT_SCENE)
	$(BLENDER) --background --python $(EXPORT_SCENE) -- '$<' '$@'

$(DIST)/SquidgeBall.pnct : SquidgeBall_chen.blend $(EXPORT_MESHES)
	$(BLENDER) --background --python $(EXPORT_MESHES) -- '$<' '$@'
