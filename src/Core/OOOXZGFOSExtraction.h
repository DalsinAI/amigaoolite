/* Native GFOS archive extraction backend for Oolite's existing OOOXZManager.
 * Caller must provide a unique, non-existing destination under its AddOns root.
 * Copyright (c) 2026 AmiCradle contributors. GPL-2.0-or-later.
 */
#ifndef OOLITE_GFOS_OXZ_EXTRACTION_H
#define OOLITE_GFOS_OXZ_EXTRACTION_H
#import <Foundation/Foundation.h>
BOOL OOGFOSExtractOXZ(NSString *archive, NSString *destination);
#endif
