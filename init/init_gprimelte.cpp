/*
   Copyright (c) 2013, The Linux Foundation. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
	* Redistributions of source code must retain the above copyright
	  notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above
	  copyright notice, this list of conditions and the following
	  disclaimer in the documentation and/or other materials provided
	  with the distribution.
	* Neither the name of The Linux Foundation nor the names of its
	  contributors may be used to endorse or promote products derived
	  from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <android-base/file.h>
#include <android-base/strings.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#include <android-base/properties.h>

#define SIMSLOT_FILE "/proc/simslot_count"

#include <android-base/logging.h>

#include "vendor_init.h"
#include "property_service.h"

#define SERIAL_NUMBER_FILE "/efs/FactoryApp/serial_no"

using android::base::GetProperty;
using android::base::ReadFileToString;
using android::base::Trim;

void property_override(char const prop[], char const value[])
{
    prop_info *pi;

    pi = (prop_info*) __system_property_find(prop);
    if (pi)
        __system_property_update(pi, value, strlen(value));
    else
        __system_property_add(prop, strlen(prop), value, strlen(value));
}

void property_override_dual(char const system_prop[], char const vendor_prop[], char const value[])
{
    property_override(system_prop, value);
    property_override(vendor_prop, value);
}

/* Read the file at filename and returns the integer
 * value in the file.
 *
 * @prereq: Assumes that integer is non-negative.
 *
 * @return: integer value read if succesful, -1 otherwise. */
int read_integer(const char* filename)
{
	int retval;
	FILE * file;

	/* open the file */
	if (!(file = fopen(filename, "r"))) {
		return -1;
	}
	/* read the value from the file */
	fscanf(file, "%d", &retval);
	fclose(file);

	return retval;
}

void set_fingerprint()
{
	property_override_dual("ro.build.fingerprint", "ro.boot.fingerprint", "google/walleye/walleye:11/RP1A.201005.004.A1/6934943:user/release-keys");
	property_override("ro.build.version.security_patch", "2020-10-05");
}

void set_cdma_properties(const char *operator_alpha, const char *operator_numeric, const char * network)
{
	/* Dynamic CDMA Properties */
	property_override("ro.cdma.home.operator.alpha", operator_alpha);
	property_override("ro.cdma.home.operator.numeric", operator_numeric);
	property_override("ro.telephony.default_network", network);

	/* Static CDMA Properties */
	property_override("ril.subscription.types", "NV,RUIM");
	property_override("ro.telephony.default_cdma_sub", "0");
	property_override("ro.telephony.get_imsi_from_sim", "true");
	property_override("ro.telephony.ril.config", "newDriverCallU,newDialCode");
	property_override("telephony.lteOnCdmaDevice", "1");
}

void set_dsds_properties()
{
	property_override("ro.multisim.simslotcount", "2");
	property_override("ro.telephony.ril.config", "simactivation");
	property_override("persist.radio.multisim.config", "dsds");
	property_override("rild.libpath2", "/vendor/lib/libsec-ril-dsds.so");
	property_override("ro.multisim.audio_follow_default_sim", "false");
}

void set_gsm_properties()
{
	property_override("telephony.lteOnCdmaDevice", "0");
	property_override("ro.telephony.default_network", "9");
}

void set_lte_properties()
{
	property_override("persist.radio.lte_vrte_ltd", "1");
	property_override("telephony.lteOnCdmaDevice", "0");
	property_override("telephony.lteOnGsmDevice", "1");
	property_override("ro.telephony.default_network", "10");
}

void set_target_properties(const char *device, const char *model)
{
	property_override_dual("ro.product.device", "ro.product.vendor.device", device);
	property_override_dual("ro.product.model", "ro.product.vendor.model", model);

	property_override("ro.ril.telephony.mqanelements", "6");

	/* check and/or set fingerprint */
	set_fingerprint();

	/* check for multi-sim devices */

	/* check if the simslot count file exists */
	if (access(SIMSLOT_FILE, F_OK) == 0) {
		int sim_count = read_integer(SIMSLOT_FILE);

		/* set the dual sim props */
		if (sim_count == 2)
			set_dsds_properties();
	}

	char const *serial_number_file = SERIAL_NUMBER_FILE;
	std::string serial_number;

	if (ReadFileToString(serial_number_file, &serial_number)) {
        	serial_number = Trim(serial_number);
        	property_override("ro.serialno", serial_number.c_str());
	}
}

void vendor_load_properties()
{
	char *device = NULL;
	char *model = NULL;

	std::string bootloader = android::base::GetProperty("ro.bootloader", "");

	if (bootloader.find("G530HXX") == 0) {
		device = (char *)"fortuna3g";
		model = (char *)"SM-G530H";
		set_gsm_properties();
		set_dsds_properties();
	}
	else if (bootloader.find("G530HXC") == 0) {
		device = (char *)"fortunave3g";
		model = (char *)"SM-G530H";
		set_gsm_properties();
		set_dsds_properties();
	}
	else if (bootloader.find("G530FZ") == 0) {
		device = (char *)"gprimeltexx";
		model = (char *)"SM-G530FZ";
		set_lte_properties();
	}
	else if (bootloader.find("G530MUU") == 0) {
		device = (char *)"fortunaltezt";
		model = (char *)"SM-G530MU";
		set_lte_properties();
	}
	else if (bootloader.find("G530MU") == 0) {
		device = (char *)"fortunalte";
		model = (char *)"SM-G530M";
		set_lte_properties();
	}
	else if (bootloader.find("G530P") == 0) {
		device = (char *)"gprimeltespr";
		model = (char *)"SM-G530P";
		set_cdma_properties("Chameleon", "310000", "10");
	}
	else if (bootloader.find("G530T1") == 0) {
		device = (char *)"gprimeltemtr";
		model = (char *)"SM-G530T1";
		set_lte_properties();
	}
	else if (bootloader.find("G530T") == 0) {
		device = (char *)"gprimeltetmo";
		model = (char *)"SM-G530T";
		set_lte_properties();
	}
	else if (bootloader.find("G530W") == 0) {
		device = (char *)"gprimeltecan";
		model = (char *)"SM-G530W";
		set_lte_properties();
	}
	else if (bootloader.find("S920L") == 0) {
		device = (char *)"gprimeltetfnvzw";
		model = (char *)"SM-S920L";
		set_cdma_properties("TracFone", "310000", "10");
	}
	else if (bootloader.find("G5309W") == 0) {
		device = (char *)"fortunaltectc";
		model = (char *)"SM-G5309W";
		set_lte_properties();
	}
	else {
		return;
	}

	/* set the properties */
	set_target_properties(device, model);
}
