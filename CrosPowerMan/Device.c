/*++

Copyright (c) 1990-98  Microsoft Corporation All Rights Reserved

Module Name:

sioctl.c

Abstract:

Purpose of this driver is to demonstrate how the four different types
of IOCTLs can be used, and how the I/O manager handles the user I/O
buffers in each case. This sample also helps to understand the usage of
some of the memory manager functions.

Environment:

Kernel mode only.

--*/


//
// Include files.
//

#include <ntddk.h>          // various NT definitions
#include <string.h>
#include <ntstrsafe.h>
#include "Device.h"
#include "Battery.h"
#include "KeyboardBacklight.h"

int comm_init_lpc(void);
int my_atoi(const char* snum);

#define NT_DEVICE_NAME      L"\\Device\\CrosPowerMan"
#define DOS_DEVICE_NAME     L"\\DosDevices\\CrosPowerMan"

#if DBG
#define CrosPowerMan_KDPRINT(_x_) \
                DbgPrint("CrosPowerMan.SYS: ");\
                DbgPrint _x_;

#else
#define CrosPowerMan_KDPRINT(_x_)
#endif

//
// Device driver routine declarations.
//

DRIVER_INITIALIZE DriverEntry;

_Dispatch_type_(IRP_MJ_CREATE)
_Dispatch_type_(IRP_MJ_CLOSE)
DRIVER_DISPATCH SioctlCreateClose;

_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
DRIVER_DISPATCH SioctlDeviceControl;

DRIVER_UNLOAD SioctlUnloadDriver;

VOID
PrintIrpInfo(
PIRP Irp
);
VOID
PrintChars(
_In_reads_(CountChars) PCHAR BufferAddress,
_In_ size_t CountChars
);

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, DriverEntry )
#pragma alloc_text( PAGE, SioctlCreateClose)
#pragma alloc_text( PAGE, SioctlDeviceControl)
#pragma alloc_text( PAGE, SioctlUnloadDriver)
#pragma alloc_text( PAGE, PrintIrpInfo)
#pragma alloc_text( PAGE, PrintChars)
#endif // ALLOC_PRAGMA


NTSTATUS
DriverEntry(
_In_ PDRIVER_OBJECT   DriverObject,
_In_ PUNICODE_STRING      RegistryPath
)
/*++

Routine Description:
This routine is called by the Operating System to initialize the driver.

It creates the device object, fills in the dispatch entry points and
completes the initialization.

Arguments:
DriverObject - a pointer to the object that represents this device
driver.

RegistryPath - a pointer to our Services key in the registry.

Return Value:
STATUS_SUCCESS if initialized; an error otherwise.

--*/

{
	NTSTATUS        ntStatus;
	UNICODE_STRING  ntUnicodeString;    // NT Device Name "\Device\SIOCTL"
	UNICODE_STRING  ntWin32NameString;    // Win32 Name "\DosDevices\IoctlTest"
	PDEVICE_OBJECT  deviceObject = NULL;    // ptr to device object

	UNREFERENCED_PARAMETER(RegistryPath);

	RtlInitUnicodeString(&ntUnicodeString, NT_DEVICE_NAME);

	ntStatus = IoCreateDevice(
		DriverObject,                   // Our Driver Object
		0,                              // We don't use a device extension
		&ntUnicodeString,               // Device name "\Device\SIOCTL"
		FILE_DEVICE_UNKNOWN,            // Device type
		FILE_DEVICE_SECURE_OPEN,     // Device characteristics
		FALSE,                          // Not an exclusive device
		&deviceObject);                // Returned ptr to Device Object

	if (!NT_SUCCESS(ntStatus))
	{
		CrosPowerMan_KDPRINT(("Couldn't create the device object\n"));
		return ntStatus;
	}

	//
	// Initialize the driver object with this driver's entry points.
	//

	DriverObject->MajorFunction[IRP_MJ_CREATE] = SioctlCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = SioctlCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = SioctlDeviceControl;
	DriverObject->DriverUnload = SioctlUnloadDriver;

	//
	// Initialize a Unicode String containing the Win32 name
	// for our device.
	//

	RtlInitUnicodeString(&ntWin32NameString, DOS_DEVICE_NAME);

	//
	// Create a symbolic link between our device name  and the Win32 name
	//

	ntStatus = IoCreateSymbolicLink(
		&ntWin32NameString, &ntUnicodeString);

	if (!NT_SUCCESS(ntStatus))
	{
		//
		// Delete everything that this routine has allocated.
		//
		CrosPowerMan_KDPRINT(("Couldn't create symbolic link\n"));
		IoDeleteDevice(deviceObject);
	}


	return ntStatus;
}


NTSTATUS
SioctlCreateClose(
PDEVICE_OBJECT DeviceObject,
PIRP Irp
)
/*++

Routine Description:

This routine is called by the I/O system when the SIOCTL is opened or
closed.

No action is performed other than completing the request successfully.

Arguments:

DeviceObject - a pointer to the object that represents the device
that I/O is to be done on.

Irp - a pointer to the I/O Request Packet for this request.

Return Value:

NT status code

--*/

{
	UNREFERENCED_PARAMETER(DeviceObject);

	PAGED_CODE();

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

VOID
SioctlUnloadDriver(
_In_ PDRIVER_OBJECT DriverObject
)
/*++

Routine Description:

This routine is called by the I/O system to unload the driver.

Any resources previously allocated must be freed.

Arguments:

DriverObject - a pointer to the object that represents our driver.

Return Value:

None
--*/

{
	PDEVICE_OBJECT deviceObject = DriverObject->DeviceObject;
	UNICODE_STRING uniWin32NameString;

	PAGED_CODE();

	//
	// Create counted string version of our Win32 device name.
	//

	RtlInitUnicodeString(&uniWin32NameString, DOS_DEVICE_NAME);


	//
	// Delete the link from our device name to a name in the Win32 namespace.
	//

	IoDeleteSymbolicLink(&uniWin32NameString);

	if (deviceObject != NULL)
	{
		IoDeleteDevice(deviceObject);
	}



}

int communicationinitialized = 0;

NTSTATUS
SioctlDeviceControl(
PDEVICE_OBJECT DeviceObject,
PIRP Irp
)

/*++

Routine Description:

This routine is called by the I/O system to perform a device I/O
control function.

Arguments:

DeviceObject - a pointer to the object that represents the device
that I/O is to be done on.

Irp - a pointer to the I/O Request Packet for this request.

Return Value:

NT status code

--*/

{
	PIO_STACK_LOCATION  irpSp;// Pointer to current stack location
	NTSTATUS            ntStatus = STATUS_SUCCESS;// Assume success
	ULONG               inBufLength; // Input buffer length
	ULONG               outBufLength; // Output buffer length
	PCHAR               inBuf, outBuf; // pointer to Input and output buffer
	//PCHAR               data = "Hello World from the Windows Kernel! ;) -- CoolStar was here.";
	//size_t              datalen = strlen(data) + 1;//Length of data including null
	PMDL                mdl = NULL;
	PCHAR               buffer = NULL;

	UNREFERENCED_PARAMETER(DeviceObject);

	PAGED_CODE();

	irpSp = IoGetCurrentIrpStackLocation(Irp);
	inBufLength = irpSp->Parameters.DeviceIoControl.InputBufferLength;
	outBufLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

	if (!inBufLength || !outBufLength)
	{
		ntStatus = STATUS_INVALID_PARAMETER;
		goto End;
	}

	//
	// Determine which I/O control code was specified.
	//

	switch (irpSp->Parameters.DeviceIoControl.IoControlCode)
	{
	case IOCTL_SIOCTL_METHOD_OUT_DIRECT:

		if (communicationinitialized == 0){
			comm_init_lpc();
			communicationinitialized = 1;
		}

		//
		// In this type of transfer, the I/O manager allocates a system buffer
		// large enough to accommodate the User input buffer, sets the buffer address
		// in Irp->AssociatedIrp.SystemBuffer and copies the content of user input buffer
		// into the SystemBuffer. For the output buffer, the I/O manager
		// probes to see whether the virtual address is writable in the callers
		// access mode, locks the pages in memory and passes the pointer to MDL
		// describing the buffer in Irp->MdlAddress.
		//
		CrosPowerMan_KDPRINT(("Called IOCTL_SIOCTL_METHOD_OUT_DIRECT\n"));

		PrintIrpInfo(Irp);


		inBuf = Irp->AssociatedIrp.SystemBuffer;

		uint32_t rawdata = 0;
		if (strcmp(inBuf, "DESIGNCAP") == 0)
			rawdata = CROSBATdesigncapacity();
		if (strcmp(inBuf, "LASTFULLCHARGE") == 0)
			rawdata = CROSBATlastfullcharge();
		if (strcmp(inBuf, "DESIGNVOLT") == 0)
			rawdata = CROSBATdesignvoltage();
		if (strcmp(inBuf, "CYCLECOUNT") == 0)
			rawdata = CROSBATcyclecount();
		if (strcmp(inBuf, "VOLTAGE") == 0)
			rawdata = CROSBATvoltage();
		if (strcmp(inBuf, "CURRENT") == 0)
			rawdata = CROSBATcurrent();
		if (strcmp(inBuf, "CHARGE") == 0)
			rawdata = CROSBATcharge();

		if (strcmp(inBuf, "GETKBLIGHT") == 0)
			rawdata = CROSKBLIGHTGetBacklight();

		if (strcmp(inBuf, "NOKBLIGHT") == 0)
			rawdata = CROSKBLIGHTSetBacklight(0);
		if (strcmp(inBuf, "HALFKBLIGHT") == 0)
			rawdata = CROSKBLIGHTSetBacklight(50);
		if (strcmp(inBuf, "FULLKBLIGHT") == 0)
			rawdata = CROSKBLIGHTSetBacklight(100);

		char *data = "";
		if (strcmp(inBuf, "FLAG") == 0) {
			char flag = CROSBATflag();
			char rawsdata[2];
			rawsdata[0] = flag;
			rawsdata[1] = '\0';
			data = rawsdata;
		}
		else if (strcmp(inBuf, "OEM") == 0){
			data = CROSBATTOEMName();
		}
		else if (strcmp(inBuf, "MODEL") == 0){
			data = CROSBATTBattModel();
		}
		else if (strcmp(inBuf, "CHEM") == 0){
			data = CROSBATTChemistry();
		}
		else if (strcmp(inBuf, "SERIAL") == 0){
			data = CROSBATTSerial();
		}
		else {
			union {
				int32_t data;
				char bytes[4];
			} ts;
			ts.data = rawdata;
			char rawsdata[5];
			rawsdata[0] = ts.bytes[0];
			rawsdata[1] = ts.bytes[1];
			rawsdata[2] = ts.bytes[2];
			rawsdata[3] = ts.bytes[3];
			rawsdata[4] = '\0';
			data = rawsdata;
		}
		size_t datalen = strlen(data) + 1;

		//
		// To access the output buffer, just get the system address
		// for the buffer. For this method, this buffer is intended for transfering data
		// from the driver to the application.
		//

		buffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);

		if (!buffer) {
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		//
		// Write data to be sent to the user in this buffer
		//

		RtlCopyBytes(buffer, data, outBufLength);

		Irp->IoStatus.Information = (outBufLength<datalen ? outBufLength : datalen);

		//
		// NOTE: Changes made to the  SystemBuffer are not copied
		// to the user input buffer by the I/O manager
		//

		break;

	default:

		//
		// The specified I/O control code is unrecognized by this driver.
		//

		ntStatus = STATUS_INVALID_DEVICE_REQUEST;
		CrosPowerMan_KDPRINT(("ERROR: unrecognized IOCTL %x\n",
			irpSp->Parameters.DeviceIoControl.IoControlCode));
		break;
	}

End:
	//
	// Finish the I/O operation by simply completing the packet and returning
	// the same status as in the packet itself.
	//

	Irp->IoStatus.Status = ntStatus;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return ntStatus;
}

VOID
PrintIrpInfo(
PIRP Irp)
{
	PIO_STACK_LOCATION  irpSp;
	irpSp = IoGetCurrentIrpStackLocation(Irp);

	PAGED_CODE();

	CrosPowerMan_KDPRINT(("\tIrp->AssociatedIrp.SystemBuffer = 0x%p\n",
		Irp->AssociatedIrp.SystemBuffer));
	CrosPowerMan_KDPRINT(("\tIrp->UserBuffer = 0x%p\n", Irp->UserBuffer));
	CrosPowerMan_KDPRINT(("\tirpSp->Parameters.DeviceIoControl.Type3InputBuffer = 0x%p\n",
		irpSp->Parameters.DeviceIoControl.Type3InputBuffer));
	CrosPowerMan_KDPRINT(("\tirpSp->Parameters.DeviceIoControl.InputBufferLength = %d\n",
		irpSp->Parameters.DeviceIoControl.InputBufferLength));
	CrosPowerMan_KDPRINT(("\tirpSp->Parameters.DeviceIoControl.OutputBufferLength = %d\n",
		irpSp->Parameters.DeviceIoControl.OutputBufferLength));
	return;
}

VOID
PrintChars(
_In_reads_(CountChars) PCHAR BufferAddress,
_In_ size_t CountChars
)
{
	PAGED_CODE();

	if (CountChars) {

		while (CountChars--) {

			if (*BufferAddress > 31
				&& *BufferAddress != 127) {

				KdPrint(("%c", *BufferAddress));

			}
			else {

				KdPrint(("."));

			}
			BufferAddress++;
		}
		KdPrint(("\n"));
	}
	return;
}


