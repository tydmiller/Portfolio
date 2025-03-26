/*File: main.c
* Author: Tyler Miller
* Date: 3-26-2025
* Description: Designed for the Raspberry Pi Zero 2 W. Turns a light on and off
* using the GPIO pins on the Raspberry Pi.
*/

/*Description: writes an assembly instruction to a certain spot in memory.
* Parameter: void* dst, the destination in memory that the instruction will be written too.
* unsigned int value, the assembly 
* Return: void
*/
void write32(void* dst, unsigned int value)
{
	unsigned int* dst_u = (unsigned int*)dst;
	*dst_u = value;
	return;
}

/*Description: Reads certain assembly instruction in memory
* Parameter: void* src, this is where in memory the instruction
* red at.
* Return: unsigned int, a number correlated to the instruction.
*/
unsigned int read32(void* src)
{
	unsigned int* src_u = (unsigned int*) src;
	return *src_u;
}

int main(void)
{
	/*GPIO Pin starts at physical spot: 0x3F200000 for firmware
	* and virtual spot: 0x7200000 for software
	*/
	unsigned int gpfsel2 = read32((void*)(0x3F200008));
	int i = 0;
	int out = 1;
	//pin 21 has to become an output in order to write
	gpfsel2 |= (1 << 3);
	write32((void*)(0x3F200008), gpfsel2);
	while(out < 100)
	{
		//write to turn pin 21 on
		write32((void*)(0x3F20001C), 1 << 21);
		while(i < 0x800000)
		{
			i++;
		}
		i = 0;
		//write to turn pin 21 off
		write32((void*)(0x3F200028), 1 << 21);
		while(i < 0x800000)
		{
			i++;
		}
		i = 0;
		//write to turn pin 21 on
		write32((void*)(0x3F20001C), 1 << 21);
		while(i < 0x800000)
		{
			i++;
		}
		out = out + 1;
	}
	return 0;
}