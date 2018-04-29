TEMPLATE = subdirs

SUBDIRS += \
    gui \
    image \
    linalg

gui.depends = image
image.depends = linalg
