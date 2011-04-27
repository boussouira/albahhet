lessThan(QT_VERSION, 4.6) {
    error("Al Bahhet requires Qt 4.6 or greater")
}

TEMPLATE = subdirs
SUBDIRS  = src
