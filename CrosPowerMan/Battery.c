#include "stdint.h"
#include "unixio.h"
#include "comm-host.h"
#include "Battery.h"

int is_battery_range(int val)
{
	return (val >= 0 && val <= 65535) ? 1 : 0;
}

static uint8_t read_mapped_mem8(uint8_t offset)
{
	int ret;
	uint8_t val;

	ret = ec_readmem(offset, sizeof(val), &val);
	if (ret <= 0) {
		fprintf(stderr, "failure in %s(): %d\n", __func__, ret);
		return -1;
	}
	return val;
}

static uint32_t read_mapped_mem32(uint8_t offset)
{
	int ret;
	uint32_t val;

	ret = ec_readmem(offset, sizeof(val), &val);
	if (ret <= 0) {
		fprintf(stderr, "failure in %s(): %d\n", __func__, ret);
		return -1;
	}
	return val;
}

static int read_mapped_string(uint8_t offset, char *buffer, int max_size)
{
	int ret;
	ret = ec_readmem(offset, max_size, buffer);
	if (ret <= 0) {
		fprintf(stderr, "failure in %s(): %d\n", __func__, ret);
		return -1;
	}
	return ret;
}

char *CROSBATTOEMName(){
	char batt_text[EC_MEMMAP_TEXT_MAX];
	read_mapped_string(EC_MEMMAP_BATT_MFGR, batt_text,
		sizeof(batt_text));
	return batt_text;
}

char *CROSBATTBattModel(){
	char batt_text[EC_MEMMAP_TEXT_MAX];
	read_mapped_string(EC_MEMMAP_BATT_MODEL, batt_text,
		sizeof(batt_text));
	return batt_text;
}

char *CROSBATTChemistry(){
	char batt_text[EC_MEMMAP_TEXT_MAX];
	read_mapped_string(EC_MEMMAP_BATT_TYPE, batt_text,
		sizeof(batt_text));
	return batt_text;
}

char *CROSBATTSerial(){
	char batt_text[EC_MEMMAP_BATT_SERIAL];
	read_mapped_string(EC_MEMMAP_BATT_TYPE, batt_text,
		sizeof(batt_text));
	return batt_text;
}

uint32_t CROSBATdesigncapacity(){
	return read_mapped_mem32(EC_MEMMAP_BATT_DCAP);
}

uint32_t CROSBATlastfullcharge(){
	return read_mapped_mem32(EC_MEMMAP_BATT_LFCC);
}


uint32_t CROSBATdesignvoltage(){
	return read_mapped_mem32(EC_MEMMAP_BATT_DVLT);
}

uint32_t CROSBATcyclecount(){
	return read_mapped_mem32(EC_MEMMAP_BATT_CCNT);
}

uint32_t CROSBATvoltage(){
	return read_mapped_mem32(EC_MEMMAP_BATT_VOLT);
}

uint32_t CROSBATcurrent(){
	return read_mapped_mem32(EC_MEMMAP_BATT_RATE);
}

uint32_t CROSBATcharge(){
	return read_mapped_mem32(EC_MEMMAP_BATT_CAP);
}

uint8_t CROSBATflag(){
	return read_mapped_mem8(EC_MEMMAP_BATT_FLAG);
}