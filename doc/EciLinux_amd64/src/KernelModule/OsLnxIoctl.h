#ifndef OSLNXIOCTL_H_INCLUDED
#define OSLNXIOCTL_H_INCLUDED

#define OSLNXKM_PCIREG1_MAP_INDEX 0
#define OSLNXKM_PCIREG2_MAP_INDEX 1
#define OSLNXKM_DEVMEM_MAP_INDEX  2
#define OSLNXKM_ADDMEM_MAP_INDEX  64
#define OSLNXKM_DMA_MAP_OFFSET    128


// For IOCTL_GETDEVICETYPE
struct getdevicetype_param
{
  uint16_t wVendorId;
  uint16_t wDeviceId;
  uint16_t wSubVendorId;
  uint16_t wSubSystemId;
};

// For IOCTL_GETDEVICEINFO
struct getdeviceinfo_param
{
  uint64_t lPciRegister1;
  uint64_t lPciRegister2;
  uint64_t lDevMemory;
  uint32_t  dwDevIrq;
};

// For IOCTL_GETDMAINFO
struct getdmainfo_param
{
  uint64_t lDmaIndex;  //in
  uint64_t lDmaOffset; //out
  uint64_t lPhysAddr;  //out
};

#define IOCTL_GETREGMAPOFFSET     _IOR ('E', 0xF2, uint64_t)
#define IOCTL_GETDATAMAPOFFSET    _IOR ('E', 0xF3, uint64_t)
#define IOCTL_GETDEVICETYPE       _IOR ('E', 0xF4, struct getdevicetype_param)
#define IOCTL_GETADDMEMMAPOFFSET  _IOR ('E', 0xF5, uint64_t)
#define IOCTL_GETDMAINFO          _IOWR('E', 0xF6, struct getdmainfo_param)
#define IOCTL_GETREG2MAPOFFSET    _IOR ('E', 0xF7, uint64_t)
#define IOCTL_GETDEVICEINFO       _IOR ('E', 0xF8, struct getdeviceinfo_param)
#define IOCTL_SETDEVICEIRQ        _IOW ('E', 0xF9, uint32_t)
#define IOCTL_GETDEVID            _IOR ('E', 0xFA, uint64_t)

#endif // OSLNXIOCTL_H_INCLUDED
