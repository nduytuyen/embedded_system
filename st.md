# STM32 Irrigation System - State Machine Diagram
**Version:** 2.1  
**Date:** November 29, 2025  
**Enhanced with Temperature Sensor Integration**

---

## State Machine Flow (Mermaid Diagram)

```mermaid
graph TD
    START["🔌 STARTUP<br/>- Khởi tạo hệ thống<br/>- I2C scan<br/>- RTC init"] -->|Init Complete| MENU

    MENU["📋 MENU<br/>CHOOSE MODE<br/>MAN/AUTO/TIMER"] 
    
    MENU -->|MANUAL Button| MANUAL["🔧 MANUAL<br/>- Bơm ON liên tục<br/>- Hiển thị Độ ẩm %<br/>- Hiển thị Nhiệt độ °C<br/>- Bơm tắt khi RESET"]
    MENU -->|AUTO Button| AUTO["⚙️ AUTO<br/>- Độ ẩm < 40% → Bơm ON<br/>- Độ ẩm ≥ 50% → Bơm OFF<br/>- Hysteresis 10%<br/>- Tự động điều khiển"]
    MENU -->|TIMER Button| TDIS["⏰ TIMER DISPLAY<br/>- Hiển thị thời gian<br/>- Chạy lịch tưới<br/>- Press TIMER → Menu"]

    MANUAL -->|RESET| MENU
    AUTO -->|RESET| MENU
    TDIS -->|RESET| MENU

    TDIS -->|TIMER Button| TMENU["📖 TIMER MENU<br/>INC/DEC để chọn<br/>>TIME SCHEDULE<br/>Press TIMER confirm"]

    TMENU -->|Chọn TIME| TTIME["⏱️ SET TIME<br/>Thiết lập giờ:phút:giây<br/>INC/DEC thay đổi<br/>TIMER → Next field<br/>TIMER 3 lần → Save"]
    TMENU -->|Chọn SCHEDULE| TSCHED["📅 SET SCHEDULE<br/>Start HH:MM + Duration<br/>INC/DEC thay đổi<br/>TIMER → Next field<br/>TIMER 3 lần → Save"]

    TTIME -->|"TIMER (Save)"| TDIS
    TTIME -->|RESET| TDIS
    TSCHED -->|"TIMER (Save)"| TDIS
    TSCHED -->|RESET| TDIS
    
    TMENU -->|RESET| TDIS

    classDef startup fill:#e3f2fd,stroke:#1976d2,stroke-width:3px,color:#000
    classDef menu fill:#fff9c4,stroke:#f57f17,stroke-width:3px,color:#000
    classDef manual fill:#c8e6c9,stroke:#388e3c,stroke-width:3px,color:#000
    classDef auto fill:#b3e5fc,stroke:#0288d1,stroke-width:3px,color:#000
    classDef timer fill:#ffe0b2,stroke:#f57c00,stroke-width:3px,color:#000
    classDef timerconfig fill:#ffccbc,stroke:#d84315,stroke-width:3px,color:#000

    class START startup
    class MENU menu
    class MANUAL manual
    class AUTO auto
    class TDIS timer
    class TMENU timer
    class TTIME timerconfig
    class TSCHED timerconfig