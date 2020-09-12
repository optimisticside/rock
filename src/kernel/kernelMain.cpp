# include <kernel/mm/physicalPageManager.h>
#include <kernel/mm/virtualPageManager.h>
#include <kernel/drivers/keyboard.h>
#include <kernel/fs/ext2/ext2.h>
#include <kernel/drivers/ahci.h>
#include <kernel/drivers/pci.h>
#include <kernel/int/syscall.h>
#include <kernel/sched/task.h>
#include <kernel/sched/hpet.h>
#include <kernel/acpi/madt.h>
#include <kernel/acpi/rsdp.h>
#include <kernel/sched/smp.h>
#include <kernel/mm/kHeap.h>
#include <kernel/int/apic.h>
#include <kernel/int/idt.h>
#include <kernel/int/gdt.h>
#include <kernel/int/tss.h>
#include <kernel/stivale.h>
#include <kernel/fs/vfs.h>
#include <lib/output.h>
#include <lib/vesa.h>
#include <lib/bmp.h> 

#include <stddef.h>

extern "C" void _init();
extern "C" void userTest() asm("userTest");
extern "C" void testDiv();

namespace kernel {

void task1();
void task2();
void task3();
void task4();
void task5();
void task6();
void task7();

void handler(const char *str) {
    kprintDS("[KDEBUG]", "lmao");
}

extern "C" void kernelMain(stivaleInfo_t *stivaleInfo) {
    _init(); /* calls all global constructors, dont put anything before here */

    physicalPageManager.init(stivaleInfo);
    kheap.init();
    virtualPageManager.init();

    idt.initIDT();

    idt.setIDTR();

    gdt.gdtInit();

    tssMain.init();
    tssMain.newTss(physicalPageManager.alloc(1) + HIGH_VMA); 

    gdt.initCore(0, (uint64_t)tssMain.tss);

    acpi.rsdpInit((uint64_t*)(stivaleInfo->rsdp + HIGH_VMA));

    madtInfo.madtInit();
    madtInfo.printMADT();
    initHPET(); 

    apic.initAPIC();

    apic.lapicTimerInit(100);

    pci.initPCI();
    ahci.initAHCI();

    initSMP();

    apic.lapicTimerInit(100);

    asm volatile(  "xor %ax, %ax\n"
                   "mov %ax, %fs"
                );

    tasks = (task_t*)kheap.kmalloc(sizeof(task_t) * 0x1000);

    readPartitions(); 

    ext2.init();

    vesa.initVESA(stivaleInfo);

    drawBMP("14569.bmp");

/*    VesaBlkGrp ok(400, 400, 3, 5, 0xffff);
    ksleep(1000);
    ok.redraw(500, 500);
    ksleep(1000);
    ok.redraw(600, 600);*/

    
    /*uint8_t cursor[] = {
                    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0,
                    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0,
                    2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0,
                    2, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0,
                    2, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0,
                    2, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0,
                    2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0,
                    2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0,
                    2, 1, 1, 2, 0, 2, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0,
                    2, 1, 2, 0, 0, 0, 2, 1, 1, 1, 1, 1, 2, 0, 0, 0,
                    2, 2, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1, 1, 2, 0, 0,
                    2, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1, 1, 2, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1, 1, 2,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 2, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0,
                   };

    VesaShape bruh(500, 500, &*cursor, 16, 16, 0xff);
    bruh.draw();
    ksleep(1000);
    bruh.redraw(400, 400);*/

    initMouse();
    
    asm volatile ("sti");

    /*    createTask(0x23, physicalPageManager.alloc(2) + 0x2000 + HIGH_VMA, 0x1b, (uint64_t)userTest, 2);
    createTask(0x10, physicalPageManager.alloc(2) + 0x2000 + HIGH_VMA, 0x8, (uint64_t)task2, 2);
    createTask(0x10, physicalPageManager.alloc(2) + 0x2000 + HIGH_VMA, 0x8, (uint64_t)task3, 2);
    createTask(0x10, physicalPageManager.alloc(2) + 0x2000 + HIGH_VMA, 0x8, (uint64_t)task4, 2);
    createTask(0x10, physicalPageManager.alloc(2) + 0x2000 + HIGH_VMA, 0x8, (uint64_t)task5, 2);
    createTask(0x10, physicalPageManager.alloc(2) + 0x2000 + HIGH_VMA, 0x8, (uint64_t)task6, 2);
    createTask(0x10, physicalPageManager.alloc(2) + 0x2000 + HIGH_VMA, 0x8, (uint64_t)task7, 2);*/
    
    for(;;);
}

void task1() {
    //uint64_t bruh = 0;
    while(1) {
        for(uint64_t i = 0; i < 100000000; i++);
      //  kprintDS("[KDEBUG]", "hi from task0 %d", bruh++);
    }
}


void task2() {
   // uint64_t bruh = 0;
    while(1) {
        for(uint64_t i = 0; i < 100000000; i++);
     //   kprintDS("[KDEBUG]", "hi from task1 %d", bruh++);
    }
}

void task3() {
  //  uint64_t bruh = 0;
    while(1) {
        for(uint64_t i = 0; i < 100000000; i++);
    //    kprintDS("[KDEBUG]", "hi from task2 %d", bruh++);
    }
}

void task4() {
 //   uint64_t bruh = 0;
    while(1) {
        for(uint64_t i = 0; i < 100000000; i++);
   //     kprintDS("[KDEBUG]", "hi from task3 %d", bruh++);
    }
}

void task5() {
//    uint64_t bruh = 0;
    while(1) {
        for(uint64_t i = 0; i < 100000000; i++);
//        kprintDS("[KDEBUG]", "hi from task4 %d", bruh++);
    }
}

void task6() {
//    uint64_t bruh = 0;
    while(1) {
        for(uint64_t i = 0; i < 100000000; i++);
 //       kprintDS("[KDEBUG]", "hi from task5 %d", bruh++);
    }
}

void task7() {
  //  uint64_t bruh = 0;
    while(1) {
        for(uint64_t i = 0; i < 100000000; i++);
  //      kprintDS("[KDEBUG]", "hi from task6 %d", bruh++);
    }
}

}