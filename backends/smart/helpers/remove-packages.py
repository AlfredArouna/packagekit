#!/usr/bin/python
#
# Copyright (C) 2007 Richard Hughes <richard@hughsie.com>
# Copyright (C) 2007 James Bowes <jbowes@redhat.com>
#
# Licensed under the GNU General Public License Version 2
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

import sys
import smartBackend


backend = smartBackend.PackageKitSmartBackend(sys.argv[2:])
backend.remove_packages(sys.argv[1], sys.argv[2])

sys.exit()