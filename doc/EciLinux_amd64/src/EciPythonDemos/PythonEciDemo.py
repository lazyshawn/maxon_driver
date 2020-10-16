#
# ECI API Demo Program
#
# converted from IXXAT example by Michael Hengler (mhengler@ixxat.de)
# 
#


from EciDemo002 import *  # @UnusedWildImport
from EciDemo003 import *  # @UnusedWildImport
from EciDemo005 import *  # @UnusedWildImport
from EciDemo101 import *  # @UnusedWildImport
from EciDemo102 import *  # @UnusedWildImport
from EciDemo105 import *  # @UnusedWildImport
from EciDemo109 import *  # @UnusedWildImport
from EciDemo10A import *  # @UnusedWildImport
from EciDemo10E import *  # @UnusedWildImport
from EciDemo111 import *  # @UnusedWildImport
from EciDemo113 import *  # @UnusedWildImport

"""

  main function

"""
if __name__ == '__main__':
    hResult = ECI_OK;

    print(">> Linux ECI API Demo program <<\n\n")

    # ECI Demo for PC-I 04 / PCI
    demo = EciDemo002();
    hResult = demo.RunDemo()

    # ECI Demo for PC-I 04 / 104
    demo = EciDemo003();
    hResult = demo.RunDemo()

    # ECI Demo for CAN-IB1x0 / PCIe (Mini), (104)
    demo = EciDemo005();
    hResult = demo.RunDemo()

    # ECI Demo for iPC-I 320 / PCI
    demo = EciDemo101();
    hResult = demo.RunDemo()

    # ECI Demo for iPC-I 320 / 104
    demo = EciDemo102();
    hResult = demo.RunDemo()

    # ECI Demo for iPC-I XC16 / PCI (PMC)
    demo = EciDemo105();
    hResult = demo.RunDemo()

    # ECI Demo for USB-to-CAN compact
    demo = EciDemo109();
    hResult = demo.RunDemo()

    # ECI Demo for USB-to-CAN II
    demo = EciDemo10A();
    hResult = demo.RunDemo()

    # ECI Demo for iPC-I XC16 / PCIe
    demo = EciDemo10E();
    hResult = demo.RunDemo()

    # ECI Demo for CAN-IB2x0 / PCIe (104)
    demo = EciDemo111();
    hResult = demo.RunDemo()

    # ECI Demo for USB-to-CAN V2
    demo = EciDemo113();
    hResult = demo.RunDemo()

    print("-> Closing Linux ECI API Demo program <-\n")

