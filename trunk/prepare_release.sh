#!/bin/sh
# Remove fake Vendor and Product ID from sourcefiles
sed -i '.norelease' 's/#define USB_\([VP]\)ID.*/#define USB_\1ID/' hid/usb_config.h
sed -i '.norelease' 's/#define USB_\([VP]\)ID.*/#define USB_\1ID/' cdc/usb_config.h
sed -i '.norelease' 's/#define USB_\([VP]\)ID.*/#define USB_\1ID/' msd/usb_config.h
sed -i '.norelease' 's/#define DFU_\([VP]\)ID.*/#define USB_\1ID/' dfu/dfu.c

# Make a release ZIP file
svn list -R | zip -u release -@

# Restore files
mv hid/usb_config.h.norelease hid/usb_config.h
mv cdc/usb_config.h.norelease cdc/usb_config.h
mv msd/usb_config.h.norelease msd/usb_config.h
mv dfu/dfu.c.norelease dfu/dfu.c

