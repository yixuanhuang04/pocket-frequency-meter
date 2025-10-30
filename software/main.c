#include <REGX52.H>

//----------------------数码管段码表----------------------
unsigned char NixieTable[] = {0x3F,0x06,0x5B,0x4F,
                              0x66,0x6D,0x7D,0x07,
                              0x7F,0x6F};  // 0~9

unsigned long fre = 0;      // 存储测得频率
unsigned char time = 0;     // 定时器2计时标志
unsigned int count = 0;     // Timer0 溢出计数

//----------------------延时子函数----------------------
void Delay(unsigned int xms)
{
    unsigned char i, j;
    while(xms--)
    {
        i = 2;
        j = 239;
        do
        {
            while (--j);
        } while (--i);
    }
}

//----------------------数码管显示----------------------
void Nixie(unsigned char Location, unsigned char Number)
{
    // 位码输出（共阴极）
    switch(Location)
    {
        case 1: P1_4=0; P1_5=1; P1_6=1; P1_7=1; break;
        case 2: P1_4=1; P1_5=0; P1_6=1; P1_7=1; break;
        case 3: P1_4=1; P1_5=1; P1_6=0; P1_7=1; break;
        case 4: P1_4=1; P1_5=1; P1_6=1; P1_7=0; break;
    }
    P0 = NixieTable[Number];  // 段码输出
    Delay(1);                  // 显示一段时间
    P0 = 0x00;                 // 清零消影
}

//----------------------定时器初始化----------------------
void timer_init(void)
{
    // Timer0 用作外部计数
    TMOD &= 0xF0; // 清零低4位
    TMOD |= 0x05; // Timer0，模式1（16位计数器）
    TH0 = 0;
    TL0 = 0;
    TR0 = 1;      // 启动计数器0
    ET0 = 1;      // 开启 Timer0 中断

    // Timer2 用作1秒定时
    RCAP2H = (65536-62500)/256; // 12MHz晶振，每次62.5ms
    RCAP2L = (65536-62500)%256;
    TH2 = RCAP2H;
    TL2 = RCAP2L;
    TR2 = 1;
    ET2 = 1;

    EA = 1; // 开总中断
}

//----------------------Timer2中断，每62.5ms----------------------
void timer2(void) interrupt 5
{
    TF2 = 0;
    time++;
    if(time == 16) // 16*62.5ms ≈ 1s
    {
        time = 0;

        TR0 = 0; // 停止计数
        fre = (unsigned long)count * 65536 + ((TH0 << 8) | TL0); // 读取频率
        count = 0;
        TH0 = 0; TL0 = 0; // 清零计数器
        TR0 = 1; // 重新启动计数器
    }
}

//----------------------Timer0中断（溢出）----------------------
void timer0(void) interrupt 1
{
    count++; // 溢出次数累加
}

//----------------------主函数----------------------
void main()
{
    timer_init(); // 初始化定时器

    while(1)
    {
        // 四位数码管显示频率（千、百、十、个位）
        Nixie(1, fre / 1000 % 10); // 千位
        Nixie(2, fre / 100 % 10);  // 百位
        Nixie(3, fre / 10 % 10);   // 十位
        Nixie(4, fre % 10);        // 个位
    }
}