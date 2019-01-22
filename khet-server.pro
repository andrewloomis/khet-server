TEMPLATE = subdirs
SUBDIRS = khetlib app
CONFIG += ordered

app.depends = khetlib
