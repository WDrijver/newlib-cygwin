#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/utsname.h>

#include <proto/exec.h>
#include <proto/wb.h>
#include <exec/execbase.h>

extern struct Library * WorkbenchBase;

int uname (struct utsname * uname) {
	if (!uname)
		return EFAULT;

	strcpy(uname->sysname, "AmigaOS");
	strcpy(uname->nodename, "alive");

	struct Library * versionLibrary = OpenLibrary("version.library", 0);
	if (versionLibrary) {
		sprintf(uname->version, "%d.%d", versionLibrary->lib_Version, versionLibrary->lib_Revision);
		CloseLibrary(versionLibrary);
	} else {
		sprintf(uname->version, "%d.%d", SysBase->LibNode.lib_Version, SysBase->LibNode.lib_Revision);
	}
	sprintf(uname->release, "%d.%d", WorkbenchBase->lib_Version, WorkbenchBase->lib_Revision);
	sprintf(uname->machine, "%04x", SysBase->ChkSum);

	return 0;
}
