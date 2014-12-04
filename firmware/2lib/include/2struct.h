/* Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Data structure definitions for verified boot, for on-disk / in-eeprom
 * data.
 */

#ifndef VBOOT_REFERENCE_VBOOT_2STRUCT_H_
#define VBOOT_REFERENCE_VBOOT_2STRUCT_H_
#include <stdint.h>

/* Algorithm types for signatures */
enum vb2_signature_algorithm {
	/* Invalid or unsupported signature type */
	VB2_SIG_INVALID = 0,

	/*
	 * No signature algorithm.  The digest is unsigned.  See
	 * VB2_GUID_NONE_* above for key GUIDs to use with this algorithm.
	 */
	VB2_SIG_NONE = 1,

	/* RSA algorithms of the given length in bits (1024-8192) */
	VB2_SIG_RSA1024 = 2,  /* Warning!  This is likely to be deprecated! */
	VB2_SIG_RSA2048 = 3,
	VB2_SIG_RSA4096 = 4,
	VB2_SIG_RSA8192 = 5,
};

/* Algorithm types for hash digests */
enum vb2_hash_algorithm {
	/* Invalid or unsupported digest type */
	VB2_HASH_INVALID = 0,

	/* SHA-1.  Warning: This is likely to be deprecated soon! */
	VB2_HASH_SHA1 = 1,

	/* SHA-256 and SHA-512 */
	VB2_HASH_SHA256 = 2,
	VB2_HASH_SHA512 = 3,
};

/*
 * Key block flags.
 *
 *The following flags set where the key is valid.  Not used by firmware
 * verification; only kernel verification.
 */
#define VB2_KEY_BLOCK_FLAG_DEVELOPER_0  0x01 /* Developer switch off */
#define VB2_KEY_BLOCK_FLAG_DEVELOPER_1  0x02 /* Developer switch on */
#define VB2_KEY_BLOCK_FLAG_RECOVERY_0   0x04 /* Not recovery mode */
#define VB2_KEY_BLOCK_FLAG_RECOVERY_1   0x08 /* Recovery mode */

/****************************************************************************/

/* Flags for vb2_shared_data.flags */
enum vb2_shared_data_flags {
	/* User has explicitly and physically requested recovery */
	VB2_SD_FLAG_MANUAL_RECOVERY = (1 << 0),

	/* Developer mode is enabled */
	VB2_SD_DEV_MODE_ENABLED = (1 << 1),

	/*
	 * TODO: might be nice to add flags for why dev mode is enabled - via
	 * gbb, virtual dev switch, or forced on for testing.
	 */
};

/* Flags for vb2_shared_data.status */
enum vb2_shared_data_status {
	/* Reinitialized NV data due to invalid checksum */
	VB2_SD_STATUS_NV_REINIT = (1 << 0),

	/* NV data has been initialized */
	VB2_SD_STATUS_NV_INIT = (1 << 1),

	/* Secure data initialized */
	VB2_SD_STATUS_SECDATA_INIT = (1 << 2),

	/* Chose a firmware slot */
	VB2_SD_STATUS_CHOSE_SLOT = (1 << 3),
};

/*
 * Data shared between vboot API calls.  Stored at the start of the work
 * buffer.
 */
struct vb2_shared_data {
	/* Flags; see enum vb2_shared_data_flags */
	uint32_t flags;

	/* Flags from GBB header */
	uint32_t gbb_flags;

	/*
	 * Reason we are in recovery mode this boot (enum vb2_nv_recovery), or
	 * 0 if we aren't.
	 */
	uint32_t recovery_reason;

	/* Firmware slot used last boot (0=A, 1=B) */
	uint32_t last_fw_slot;

	/* Result of last boot (enum vb2_fw_result) */
	uint32_t last_fw_result;

	/* Firmware slot used this boot */
	uint32_t fw_slot;

	/*
	 * Version for this slot (top 16 bits = key, lower 16 bits = firmware).
	 *
	 * TODO: Make this a union to allow getting/setting those versions
	 * separately?
	 */
	uint32_t fw_version;

	/*
	 * Status flags for this boot; see enum vb2_shared_data_status.  Status
	 * is "what we've done"; flags above are "decisions we've made".
	 */
	uint32_t status;

	/**********************************************************************
	 * Temporary variables used during firmware verification.  These don't
	 * really need to persist through to the OS, but there's nowhere else
	 * we can put them.
	 */

	/* Root key offset and size from GBB header */
	uint32_t gbb_rootkey_offset;
	uint32_t gbb_rootkey_size;

	/* Offset of preamble from start of vblock */
	uint32_t vblock_preamble_offset;

	/*
	 * Offset and size of packed data key in work buffer.  Size is 0 if
	 * data key is not stored in the work buffer.
	 */
	uint32_t workbuf_data_key_offset;
	uint32_t workbuf_data_key_size;

	/*
	 * Offset and size of firmware preamble in work buffer.  Size if 0 if
	 * preamble is not stored in the work buffer.
	 */
	uint32_t workbuf_preamble_offset;
	uint32_t workbuf_preamble_size;

	/*
	 * Offset and size of hash context in work buffer.  Size if 0 if
	 * hash context is not stored in the work buffer.
	 */
	uint32_t workbuf_hash_offset;
	uint32_t workbuf_hash_size;

	/*
	 * Current tag we're hashing
	 *
	 * For new structs, this is the offset of the vb2_signature struct
	 * in the work buffer.
	 *
	 * TODO: rename to workbuf_hash_sig_offset when vboot1 structs are
	 * deprecated.
	 */
	uint32_t hash_tag;

	/* Amount of data we still expect to hash */
	uint32_t hash_remaining_size;

} __attribute__((packed));

/****************************************************************************/

/* Signature at start of the GBB
 * Note that if you compile in the signature as is, you are likely to break any
 * tools that search for the signature. */
#define VB2_GBB_SIGNATURE "$GBB"
#define VB2_GBB_SIGNATURE_SIZE 4
#define VB2_GBB_XOR_CHARS "****"
/* TODO: can we write a macro to produce this at compile time? */
#define VB2_GBB_XOR_SIGNATURE { 0x0e, 0x6d, 0x68, 0x68 }

/* VB2 GBB struct version */
#define VB2_GBB_MAJOR_VER      1
#define VB2_GBB_MINOR_VER      2
/* v1.2 - added fields for sha256 digest of the HWID */

/* Flags for vb2_gbb_header.flags */
enum vb2_gbb_flag {
	/*
	 * Reduce the dev screen delay to 2 sec from 30 sec to speed up
	 * factory.
	 */
	VB2_GBB_FLAG_DEV_SCREEN_SHORT_DELAY = (1 << 0),

	/*
	 * BIOS should load option ROMs from arbitrary PCI devices. We'll never
	 * enable this ourselves because it executes non-verified code, but if
	 * a customer wants to void their warranty and set this flag in the
	 * read-only flash, they should be able to do so.
	 */
	VB2_GBB_FLAG_LOAD_OPTION_ROMS = (1 << 1),

	/*
	 * The factory flow may need the BIOS to boot a non-ChromeOS kernel if
	 * the dev-switch is on. This flag allows that.
	 */
	VB2_GBB_FLAG_ENABLE_ALTERNATE_OS = (1 << 2),

	/*
	 * Force dev switch on, regardless of physical/keyboard dev switch
	 * position.
	 */
	VB2_GBB_FLAG_FORCE_DEV_SWITCH_ON = (1 << 3),

	/* Allow booting from USB in dev mode even if dev_boot_usb=0. */
	VB2_GBB_FLAG_FORCE_DEV_BOOT_USB = (1 << 4),

	/* Disable firmware rollback protection. */
	VB2_GBB_FLAG_DISABLE_FW_ROLLBACK_CHECK = (1 << 5),

	/* Allow Enter key to trigger dev->tonorm screen transition */
	VB2_GBB_FLAG_ENTER_TRIGGERS_TONORM = (1 << 6),

	/* Allow booting Legacy OSes in dev mode even if dev_boot_legacy=0. */
	VB2_GBB_FLAG_FORCE_DEV_BOOT_LEGACY = (1 << 7),

	/* Allow booting using alternate keys for FAFT servo testing */
	VB2_GBB_FLAG_FAFT_KEY_OVERIDE = (1 << 8),

	/* Disable EC software sync */
	VB2_GBB_FLAG_DISABLE_EC_SOFTWARE_SYNC = (1 << 9),

	/* Default to booting legacy OS when dev screen times out */
	VB2_GBB_FLAG_DEFAULT_DEV_BOOT_LEGACY = (1 << 10),

	/* Disable PD software sync */
	VB2_GBB_FLAG_DISABLE_PD_SOFTWARE_SYNC = (1 << 11),
};

struct vb2_gbb_header {
	/* Fields present in version 1.1 */
	uint8_t  signature[VB2_GBB_SIGNATURE_SIZE]; /* VB2_GBB_SIGNATURE */
	uint16_t major_version;   /* See VB2_GBB_MAJOR_VER */
	uint16_t minor_version;   /* See VB2_GBB_MINOR_VER */
	uint32_t header_size;     /* Size of GBB header in bytes */
	uint32_t flags;           /* Flags (see enum vb2_gbb_flag) */

	/* Offsets (from start of header) and sizes (in bytes) of components */
	uint32_t hwid_offset;		/* HWID */
	uint32_t hwid_size;
	uint32_t rootkey_offset;	/* Root key */
	uint32_t rootkey_size;
	uint32_t bmpfv_offset;		/* BMP FV */
	uint32_t bmpfv_size;
	uint32_t recovery_key_offset;	/* Recovery key */
	uint32_t recovery_key_size;

	/* Added in version 1.2 */
	uint8_t  hwid_digest[32];	/* SHA-256 of HWID */

	/* Pad to match EXPECETED_VB2_GBB_HEADER_SIZE.  Initialize to 0. */
	uint8_t  pad[48];
} __attribute__((packed));

/* The GBB is used outside of vboot_reference, so this size is important. */
#define EXPECTED_VB2_GBB_HEADER_SIZE 128

#endif  /* VBOOT_REFERENCE_VBOOT_2STRUCT_H_ */
