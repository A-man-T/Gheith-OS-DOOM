
#ifndef _IRQ_H_
#define _IRQ_H_
#include "stdint.h"
#include "debug.h"
#include "idt.h"

class IRQ {
    public:
        static void cpu_write_IO_Apic(void *ioapicaddr, uint32_t reg, uint32_t value) {
            uint32_t volatile *ioapic = (uint32_t volatile *)ioapicaddr;
            ioapic[0] = (reg & 0xff);
            ioapic[4] = value;
        }

        static void map_vector_IRQ(int vector, int irq_id){
            uint32_t low_bits = vector;
            uint32_t high_bits = 0;
            cpu_write_IO_Apic((void*)kConfig.ioAPIC, 0x10 + (irq_id * 2), low_bits);
            cpu_write_IO_Apic((void*)kConfig.ioAPIC, 0x11 + (irq_id * 2), high_bits);
        }

        static void map_IRQ(int irq_id, int32_t vector, uint32_t handler) {
            map_vector_IRQ(vector, irq_id);
            IDT::interrupt(vector, (uint32_t)handler); 
        }
};
#endif

