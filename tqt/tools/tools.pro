TEMPLATE	= subdirs
no-png {
    message("Tools not available without PNG support")
} else {
    SUBDIRS		= assistant/lib \
		      designer \
		      assistant \
		      linguist
    unix:SUBDIRS	+= qtconfig \
			tqtmergetr \
			tqconfig \
			msg2tqm \
			qembed
}

CONFIG+=ordered
REQUIRES=full-config
