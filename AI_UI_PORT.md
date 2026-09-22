本轮迁移：AI UI 与四通道参数保存

- 主页面为四通道 BSD / Pallet / SETUP。BSD 与 Pallet 互斥，SETUP 打开当前通道所选算法。标题：默认、法文、德文为 AI，中文为“算法”；德文通道名称为 Kam1～Kam4。
- BSD / Pallet 保留四语言文字。BSD 背景 664×194，位置 (128,140)；Pallet 复用原文字图片，按行裁取并排列为六行滑条。显示区域仍为 1024×600。
- BSD / Pallet 二级菜单不显示顶部通道标题。
- BSD、Pallet 各四张状态图，均为 107×42，与现有 Cursor SETUP 一致。绿底表示焦点，绿字表示算法选中。
- Pallet 页面已移除加减按钮和独立数值框，数值居中显示在绿色滑块中。四张旧加减图片及对应资源条目已删除。
- BSD 的人检测、车辆检测、报警音、触发输出、Cursor 使用当前通道的原有配置。
- Pallet 顺序为 Delay Turn Off、Rate Range、Flow Frame Interval、Flow Pause Duration、Flow Run Duration、Sensitivity。左右键选择一行，Execute 进入/退出调整；调整时左右键支持长按连续步进，各参数到达边界后停止。六行均可点击、拖动，触摸吸附到最近的合法步长。

- AI、BSD、Pallet 的触摸返回键先显示高亮，再沿用原保存/退出逻辑。Pallet 滑块、滑条和选中框均复用 System Dimmer 的不透明资源；486×56 选中框只框住右侧滑条，不框住翻译文字。

默认值与范围

| 字段 | 默认值 | 范围 / 步长 |
| --- | --- | --- |
| tDelayTurnOFF | 6 | 2–10 / 2 |
| tFlowFrameInterval | 0 | 0–5 / 1 |
| tRateRange | 2 | 1–6 / 1 |
| tFlowPauseDuration | 3 | 0–5 / 1 |
| tFlowRunDuration | 1 | 1–5 / 1 |
| tDectability | 52 | 0–99 / 1 |

Flash

每路 UI_CamStatus_t 的原预留区使用 8 字节，保存算法选择、6 个参数及版本标记，剩余预留空间从 163 调整为 155 字节，原结构大小和既有字段偏移不变。默认值与合法性处理分别接入 UI_ResetUIParameter / UI_CheckUIParameter，保存与加载沿用 UI_UpdateDevStatusInfo / UI_LoadDevStatusInfo。退出 SETUP 时保存。旧版本或非法新增配置只恢复该通道的新增字段，不清除配对信息。

资源与验证

- OSDdata_1.ini、OSD2Image_Table[WSVGA].h、四张 AI_PALLET_SETUP 图片、根目录和 ConfigGen 目录的 OSDImage.dat 已配套更新。四张旧加减图标已从 BMP、资源表、INI 和两份 DAT 中删除，后续资源索引已同步调整。资源数量为 944，DAT 为 13,470,216 字节；本次删除保留了其余 944 张资源的像素和非地址元数据。
- 六行滑条位置从 (508,88) 开始，尺寸 440×48；选中框从 (486,84) 开始，尺寸固定为 486×56。文字与滑条行距统一为 68，四语言文字图片为 440×388；文字 X 坐标由资源配置（默认 252、法文 112、德文 128、中文 262），绘制函数不再区分宽框或单独裁剪最后一行。
- 本次临时 C 验证通过：1,064 个按键边界组合、22,128 个点击/拖动组合、全部合法值的滑块中心点击、48 个四语言重绘/不透底组合，以及四通道入口、BSD 开关、Cursor 通道、菜单焦点和三页返回高亮。配置结构、默认值与合法性校验未因布局改动而改变。
- 本次 SUBSUBMENU 文件通过 GCC 语法检查，保留原有三处隐式声明告警；临时菜单验证程序使用实际菜单函数和 DAT 资源，通过 GCC 编译及运行。未完成 ARMCC/Keil 全量编译、实际 ConfigGen 生成、固件烧录或真机验证。
- AI_UI_PREVIEW.png 为根据实际 UI 绘制调用与 BMP 合成的布局预览，不是真机截图。

后续音频、算法通信和重启确认迁移见下文。新图片资源与代码需要配套重新编译打包，现有发布 BIN 未更新。

2026-09-20：音频、通道同步和栈板检测标志

- 点击不同的 BSD/Pallet 选项立即弹出四语言确认框，默认选否。确认前不修改通道；选否、返回或取消保留原选择。选是后保存并同步四路配置。选择新的栈板通道时，原栈板通道同时恢复 BSD；重复点击已选算法不弹窗、不保存。加载旧配置时仅保留第一个栈板通道。
- SETUP 参数和栈板引导线在退出设置时保存、同步。1126 比较运行中的通道映射，仅参数变化不重启。完整配置通过 `FF CC` 的 18 字节帧一次发送；四路算法值位于字节 12～15（0=BSD、1=Pallet），字节 3 是参数所属的栈板通道，CRC 覆盖长度字段至最后一路算法。应答 `FF CC 08 00 mask status CRC_L CRC_H`，mask 的 bit0～3 对应 CAM1～4，status 0=已保存/准备重启、1=已生效、2=非法选择、3=保存失败。未收到匹配的成功应答时，每秒重发。每次 1126 启动均请求重新对齐配置。
- `HD900-alg/pd/pd.cpp` 使用 `/root/config/SonixAI.ini` 保存 `PalletMask`，区分 Sonix 的四路 SOURCE_ID 与内部算法通道；首次运行兼容旧 `pdsModel=18` 的 CAM1 栈板模式。参数保存在原 `PalletConfig.ini`，切入栈板前先导入、保存参数。配置未变不重复写盘、不重复重启；保留旧 14/15 字节接收兼容。重启沿用现有机制：配置落盘、应答后退出 alg，由 `check.sh` 拉起，未执行整板 reboot。
- 音频包 `aduexamp_flow.aud32` 从 FlowAnalyzer 原样迁入，包含三级音频，每段 1,920 字节；`ConfigGen.INI` 的音频输入已指向该文件。按各通道全部目标的最高报警等级播放索引 0/1/2，每秒调度且等待上一段结束；空目标只清本通道，断流一秒后清除失效报警。Pallet 报警不受 BSD 的检人/检车开关影响。最终固件仍需用 ConfigGen 合入该音频包，不能只替换 OSDImage.dat。
- 使用用户提供的 `Common/Pallet.bmp`（50×50）与 `Pallet_half.bmp`（25×25），复用车辆标志位置；单画面使用大图，双画面/四分割使用小图。检测帧类别 bit2 表示栈板，与人、车以及原灯控 bit4 分开。栈板与车辆同时检出时，该位置优先显示栈板标志。
- 新增四张确认框 BMP，复用升级框的四语言是/否按钮。OSD2 914 张，加 OSD1 共 950 张；两份 DAT 均为 13,995,677 字节，原 944 张图片的像素、尺寸、坐标及调色板保持不变。代码、资源表、INI、DAT 必须配套打包。

验证：320 组确认/取消、触摸及通道切换场景通过；160 份由实际 Sonix 发送函数产生的帧通过 1126 接收、CRC、持久化/启动读取、重复发送、错误输入和保存失败验证；实际 Sonix 串口接收状态机通过配置应答、栈板标志、空帧 CRC 和启动请求验证；三级音频优先级、空帧与断流处理通过。四个修改的 C 文件完成 GCC 兼容语法检查，与 HEAD 对照未新增诊断；仅临时副本适配原有 ARMCC 数组初始化和 case 后声明。950 条资源及三段音频包结构校验通过。

尚未完成 ARMCC/1126 全量交叉编译、烧录和真机验证。1126 工程缺少上层 SDK 的 Makefile.param、公共头文件等构建依赖；需在完整 SDK 中编译，并确认板端 `check.sh` 正在监护 alg。真机重点检查四路轮换栈板、取消保留旧通道、仅改参数不重启、四分割图标、双芯片分别断电重启和三级提示音。临时测试文件不保留。

光流 Laser / LED 灯控补充

- 接入 1126 检测帧 `pc_flag` 的 bit4（`0x10`）。整帧 CRC 校验成功后，Sonix 为实际 Pallet 通道刷新 300 ms 倒计时；即使目标数为零、菜单已打开，也处理灯控位。bit4 清零或数据中断后不再续时，倒计时结束时关闭临时点亮的 Laser 和 LED。
- 灯控在独立的 `UI_AILampThread` 执行，首次开灯或倒计时归零后重新收到开灯数据时，通过 `UI_AI_LAMP_SIGNAL`（`0x100`）唤醒；连续开灯帧只刷新倒计时，不逐帧发信号。无信号时阻塞等待最多 50 ms，再检查超时关灯和重试。菜单事件任务恢复阻塞等待事件，串口接收任务和毫秒回调不等待摄像头应答。灯控不再等待菜单事件处理结束，但实际执行时间仍受共用命令锁和 TWC 应答耗时影响。
- `UI_SendLaserorLedToCAM` 与 `UI_SendRequestToCAM` 共用 `osUI_CamCmdMutex`，从发送到应答结束全程持锁，保护共用的 `tosUI_Notify`；camera 设置页通过调用发送函数自动获取锁。灯控取得锁后重新检查当前通道、倒计时和菜单常开设置，避免等待期间过期的开灯请求被执行。临时灯状态也在锁内更新。灯控唤醒信号与摄像头应答信号 `0x66` 分开，发送前清理遗留应答，检查应答时按信号位判断。只在灯状态需要改变时发送；失败保留未知状态并重试。
- 沿用旧工程的菜单优先规则：Laser / LED 设置为常开时保持常开，自动灯控不改写保存的菜单配置。仅支持版本标识含 `LAS` 的摄像头。四路分别记录状态，切换 Pallet 通道、待机、升级或配置重新同步时撤销旧的临时开灯；断线时停止发送，重连后再同步。
- 修改 `UART4AI.c`、`UI_BUCCU[WSVGA].c/.h` 和 `UI.c`，1126 已有 `bLampSwitch` 输出，无需改报文。独立任务改动通过 35 组临时验证，包含实际接收函数的信号通知、四通道常开组合、主机双线程并发发送、独立任务信号唤醒/超时检查、发送及应答失败释放锁、唤醒与应答信号同时存在、等待期间配置变化和倒计时过期；三个 C 文件使用项目宏和头文件完成 GCC 兼容语法检查，与修改前对照无新增诊断。未完成 ARMCC 全量编译与真机灯控验证。


2026-09-21：旧版本核对与配置生效日志

对照范围：`HD900-alg/pd/pd.cpp`、`pd/pd_bak/pd_20250403_cjy.cpp`、`pd_pallet.cpp`、`pd_pallet_251009.cpp`，以及旧 `HDW297RS-AI-FCC-FlowAnalyzer` 的 UART4AI、主 UI、菜单、引导线校准、音频和灯控引用。备份并不是同一版栈板算法：2025-04-03 使用旧 `VehicleStateAnalysis`；`pd_pallet.cpp` 使用 RGB_PC_PALLET 检测模型；包含六项参数、标定线和光流灯控的是 `pd_pallet_251009.cpp`。

| 功能 | 当前核对结果 |
| --- | --- |
| 六项栈板参数、标定线坐标换算、退出设置后发送 | 已接入；Sonix 的 Y+2 按 600→352 换算，Range 乘 10/20/30，Sensitivity 除以 100，与旧栈板版一致。 |
| 配置运行时更新 | 与旧栈板版一致，`SaveParams` 和 `PalletStateAnalysis` 使用同一个 `g_cPalletAnalyzer`；调用共用算法锁。此次把成功后的参数快照更新也放进锁内。 |
| 栈板目标、报警等级、单/分割坐标映射及横线 | 已接入；Pallet 使用库返回的每目标等级。人车开关不再屏蔽 Pallet；栈板图标新增独立 bit2。 |
| 三级音频、等级变化立即重新调度、空目标停止 | 已接入；当前按有效通道的最高等级仲裁，并增加断流过期，避免一个空通道停掉其他通道的声音。 |
| 光流 bit4 自动 Laser/LED、300 ms 保持、常开设置优先 | 已接入独立灯控任务，支持实际选中的栈板通道。 |
| 校准页面临时照明 | 已于 2026-09-22 接入现有独立灯控任务，按当前校准页面和通道请求临时开灯；退出撤销，保留手动常开及光流灯控，详见文末。 |
| 2025-04-03 的静止车辆不发送检测结果 | 当前未保留此旧 BSD 外层过滤；后续 `pd_pallet_251009.cpp` 已注释该条件，改用 `PalletStateAnalysis`。不要直接给四路 BSD 加此过滤。 |
| 旧版栈板推理耗时打印 | 未迁移：`pd_pallet_251009.cpp` 在 `bDetectPallet` 时逐帧打印耗时。这是调试输出，不影响栈板检测。 |

另一个已有通信限制：1126 检测报文最多可组 50 个目标，而 Sonix 的 `DATA_LENGTH_MAX` 为 30；超过 30 的整帧会被 Sonix 接收状态机拒绝，不是自动显示前 30 个。单画面实际显示上限仍是 10，双/四画面每通道为 5。这次审查仅记录该边界，未改变目标数量协议。旧工程也有超限拒收分支。

本次 `pd.cpp` 增加统一 `[AI_CFG]` 日志，沿用项目 `print_level`；板端应启用 `SV_INFO` 输出，查看完整 alg 日志或筛选 `[AI_CFG]`。

| 标记 | 含义 |
| --- | --- |
| `RX` | CRC 通过后的原始参数，CAM1～4 与 SOURCE_ID=0～3 同时打印；line600 是 UI 坐标，sensitivity 是百分数。 |
| `MAP` | 请求、当前运行、已保存的 PalletMask，及请求的四路 BSD/Pallet；mask=0 全 BSD，1/2/4/8 分别为 CAM1/2/3/4 栈板。 |
| `PARAM` | SaveParams 成功、参数相同跳过写盘，或全 BSD 忽略栈板参数。line352 是换算后送入库的标定线。 |
| `SAVE` / `RESTART` | 通道映射落盘，随后退出 alg，由 check.sh 拉起；不是整块 1126 执行 reboot。 |
| `BOOT` / `QUERY` | 新进程实际读取的四路映射、ImportParams 结果，以及启动时向 Sonix 请求完整配置。 |
| `RUN` | 该 SOURCE_ID 成功完成推理；启动后每路一次，栈板参数实际变化后再打印一次。包含 Pallet 当前参数快照、目标数、灯控位。无图像或推理失败不打印成功。 |
| `ACK` / `REJECT` / `ERROR` | 配置应答、CRC/选择/参数拒收、导入/加锁/保存/重启线程失败。 |

板端判断方法：

- **只改 SETUP 参数**：退出页面后应看到 RX 的新值、PARAM 的 `SaveParams 成功`、MAP 的无需新增重启、随后同一 CAM 的 RUN 新参数。重复提交相同值只看到参数相同，不再刷 RUN。
- **把栈板从 CAM1 改为 CAM3**：先看 MAP 的 `request=0x04 running=0x01`，再看 SAVE、RESTART；重新启动后 BOOT 应为 CAM3=Pallet、CAM1=BSD，收到各通道图像后分别有对应 RUN。只看到 SAVE 或 ACK status=0 不能证明新通道已运行。
- 通道切换沿用先更新栈板参数、后保存映射并重启的流程。在退出前旧栈板通道仍可能推理，日志中的 running/BOOT/RUN 用来区分此过渡阶段。
- RUN 中参数是最近成功传给 ImportParams/SaveParams 的应用层快照，**不是算法库内部读回**。仓库仅有接口头文件，没有 `libopticalflow` 的实现或读取运行参数接口；日志证明同一实例更新调用及后续推理成功，最终阈值、延时等实际效果仍须板端验证。

验证：提取实际配置接收、CRC、保存映射、重启调度、启动读取和 RUN 日志代码，配合工程真实 OpticalFlow 头文件与模拟 SDK/系统调用，G++ C++11 编译并通过 49 组临时场景（25 组全部有效映射互切/重复提交、各可编辑参数、旧 14/15 字节、非法输入、CRC 错误、保存/导入/加锁/线程创建失败、启动兼容、推理失败不误报及日志只输出一次）。无编译告警。临时文件已清除；没有新增正式测试。缺少上层 Makefile.param 和完整 1126 SDK，未做全量交叉编译或真机验证。

2026-09-21：升级握手排查

- Sonix 的升级报文与 AI 配置共用 `FF CC` 包头，但格式不同：配置第三字节是总长 14/15/18；升级应答是三字节 `FF CC cmd`，Sonix 当前发送的命令为 00/01/04/98/99。升级文件本体由 `KNL_UsbdFwUpgradeThread` 经 USB/UVC 传输。
- 已复现 `pd.cpp` 的解析问题：`FF CC 98/99` 被误当作 152/153 字节长帧，后续配置被吞入未完成报文。现在区分三字节升级应答，输出 `[AI_UPGRADE]` 并复位接收状态；其他不支持的 FF CC 长度及时拒收。此改动修复解析，不等于增加了升级文件接收和升级程序启动入口。
- 用户提供的 `autoUpdate` 代码从 `/tmp` 的已接收文件开始校验、覆盖，结束后循环发送 `FF CC 99`，等 Sonix 应答后才调用 `ALG_WAKE_UP_SH`。其 `uart_recieve()` 仅检查每次 `read()` 的前三字节，已复现拆包及前面粘有其他数据时漏掉 99 应答，导致无法结束等待。
- 对用户提供代码生成最小补丁 `HD900-alg/upgrade/autoUpdate_uart_receive.patch`：逐字节接收并保留半包，跳过完整 AI 配置帧，识别结束应答。实际 autoUpdate 工程路径尚未提供，**该补丁尚未应用到实际升级工程**。仓库现有 `upgrade/upgrade.cpp` 是另一份代码，不能据此认为板端升级程序已修好。
- 仍待定位的是前级收包/启动流程：谁发送 `FF CC 00/01/04` 请求 Sonix 文件、谁读取 UVC 并将包准备到 `/tmp`，以及何时让出串口给 autoUpdate。若 alg 和升级程序同时读取 `/dev/ttyS4`，会竞争读取应答；需板端启动脚本或运行进程信息确认，不能仅凭此代码断定发生了竞争。
- 临时验证：原版两个失败场景均已复现；实际 pd 串口接收函数通过 118 组配置/升级应答粘包、拆包、版本查询、非法长度和负载内包头验证；autoUpdate 接收补丁通过 52 组拆包、粘包、配置跳过及错误应答验证。使用 G++ 与模拟串口调用，不执行升级/系统覆盖；未做完整交叉编译和板端升级。临时目录 `.upgrade_check` 及其 11 个测试文件已按用户要求删除。

2026-09-21：旧根文件系统缺少栈板配置时自动补建

- `HD900-alg/pd/pd.cpp` 增加 `pd_CheckPalletConfig()`。启动时已有 Pallet 通道，或运行中收到启用 Pallet 的 Sonix 配置，在 `ImportParams` 前检查 `/root/config/PalletConfig.ini`；只有文件不存在（ENOENT）才按用户提供的完整 INI 模板补建。先写 `.tmp`、flush/fsync、关闭，再 rename 到正式路径。已有文件不重置；访问失败、创建失败或导入失败继续按失败处理，不误报配置成功。
- 模板包含 GlobalConfig、FlowConfig、PalletConfig 共 22 项，模型路径为 `/root/model/flownet_pre.rknn`、`/root/model/PC_PalletDet_448.rknn`，图像为 608×352；其余值与用户提供内容一致。首次导入后，由原接收逻辑用 Sonix 报文更新六项参数及标定线；相同参数不调用 SaveParams。全 BSD 保持不依赖栈板配置。
- `g_u8SavedAlgorithmMask=0xff` 表示未读到有效 `SonixAI.ini`；只有成功读到合法映射或写入成功才记录已保存值。修复文件缺失/无效但 Sonix 选择与旧 pdsModel 回退值相同时，不创建通道文件的问题。文件有效且映射相同仍跳过写入，运行映射相同不重启。
- 新增 `[AI_CFG][CREATE]` 补建成功日志；后续仍由 PARAM、SAVE、BOOT、RUN 判断保存及运行状态。模型文件和算法库不会由 INI 自动生成，仍需随升级包提供。

验证：嵌入模板逐项比对用户提供内容一致；提取实际检查/创建、参数收包、映射保存及启动读取代码，使用工程真实 OpticalFlow 头文件及模拟算法库/SDK接口，G++ C++11 编译并通过 33 组缺文件、已有文件、同值跳过、参数变化、四路启动、旧 14/15 字节、非法输入、写入/导入失败及重试场景；修改前两个缺文件问题均已复现。仅屏蔽原 OpticalFlowCommon.h 构造函数已有的未使用参数告警，无新增诊断。保留源码 CRLF，没有新增正式测试文件。未进行完整 1126 SDK 交叉编译或真机验证。

2026-09-21：Guideline 分屏与 BSD 法德文资源修正

- 主界面 guideline 改由 UI_DrawBSDRange 画到 OSD1，删除 UI_DrawNoSignalIcon 中旧的 OSD2 画线分支。复用现有纯红色横线资源，单画面使用原坐标，双画面横向减半，四分割横纵减半并按 tCamViewPool 对应窗口偏移；算法清框沿用 OSD1 底图恢复，不会擦掉 guideline。
- Pallet 通道始终显示自己的 guideline，不依赖 GuideLineEnable、BSD parkingline、BSD range 或检人/检车开关；该通道不再叠加 BSD 检测区及普通/AUTO 倒车线。BSD 通道按原开关显示倒车线，guideline 按自身开关显示。
- AI_BSD_SETUP_FR.bmp / AI_BSD_SETUP_GEN.bmp 第一行两项文字放大，与下方标签字号匹配；下方标签像素保持不变。两张 BMP 从 664×194 扩为 724×194。AI_BSD_SWITCH_X 从 338 调至 398，第二列对应 852；绘制、焦点和触摸命中共用该宏，控件仍为 107×42。
- INI 和根目录、ConfigGen 目录的两份 OSDImage.dat 配套更新；资源数仍为 950，DAT 为 14,018,957 字节，其余 948 张资源的像素、尺寸、位置和保留字段、整个调色板保持不变。现有固件 BIN 未更新，需将代码和资源一起重新编译打包。
- 图像使用内置 imagegen 编辑，提示要求：只将法德文第一行两项放大至下方 24px 字体大小；保留准确文字、黑底白字及其余标签，必要时宽度扩至 724。提示文字为法文 “Détection de personne” / “Détection de véhicules”，德文 “Personenerkennung” / “Fahrzeugerkennung”。仅裁取生成结果的四个标签，按原布局缩放、套用已有灰度调色板，再合入原 BMP。

验证：实际绘制函数经 GCC 编译，138,240 组通道排列/分屏/算法/开关组合及 216 组尺寸/边界组合通过；使用实际 UI_ClearAIBoxArea 和 OSD_EraserImg2_NoUpdate 验证三个视图清框后 guideline 像素恢复。主 UI 与 BSD 菜单完成 GCC 兼容语法检查，与修改前对照均无新增告警（临时副本仅适配两个原有 ARMCC 数组初始化）；950 条资源结构、两份 DAT 一致性及法德文菜单布局核对通过。临时验证文件已删除，没有新增正式测试。未进行 ARMCC 全量编译、烧录或真机验证。

2026-09-21：栈板目标改画中心横线

- UI_EventDrawBox 对 Pallet 通道使用接收坐标的 (y1+y2)/2，显示为 4 像素高的横线，横向继续沿用目标 x1～x2 及已有小目标/窗口边界处理。先计算中心，再执行原矩形扩边，避免小目标扩大或贴边平移改变横线中心；横线在当前分屏区域内限位。
- COMMON_SRC/OSD/OSD.c 放宽绘制高度下限至 4 像素，复用原颜色索引及绘制接口。动态目标横线仍在 OSD2，标定 guideline 保留在 OSD1。颜色和是否显示继续使用 1126 返回的报警等级；本次未修改 1126 算法、报警计算或串口协议，BSD 通道仍画矩形框。
- 临时验证：提取实际 UI_EventDrawBox、窗口定位、边界处理和 OSD 像素绘制函数，通过 1,400 组四通道/单双四画面/奇偶中心/小目标/边界及异常坐标/颜色组合；BSD 输出像素逐字节与本次修改前一致；多目标和空帧清除通过。两个 C 文件完成 GCC 兼容语法检查，无新增诊断（UI 保留原有 1 个告警，OSD 无告警）。临时文件已清理。未完成 ARMCC 全量编译或真机验证。

2026-09-22：Pallet 设置页翻译、布局和开关

- 左侧保留六项参数及原范围、步长，滑条从 440×48 缩短为 200×48，焦点框从 486×56 缩短为 246×56，行距仍为 68。右侧依次为 Pallet Detection、Trigger Out、Alarm Sound。触摸仅在 TOUCH_PRESSDOWN 时选中、改值，并沿用 Camera 每三次长按事件处理一次的降速方式；单击滑条不改变焦点或数值。遥控支持全部九项。
- Trigger Out 和 Alarm Sound 直接读写当前通道的 ubBSDTriggerOut / ubIsEnableBSDALARM；报警声音判断对 Pallet 也应用该开关。四种语言的这两项文字直接裁取当前 AI_BSD_SETUP 对应第二行资源，字号、文字、像素保持一致，FR/GEN 均不换行。
- 新增每通道 ubDetectPalletFlag，ON 检测、OFF 停止。占用原保留区一个字节，UI_CamStatus_t 总大小和旧字段偏移不变。配置版本升为 2，版本 1 只补设检测开关为 ON，保留原算法选择及六项参数；恢复默认也设为 ON。
- Sonix 在既有 SEI 检测模式字段中对 Pallet 发送 ON=3 / OFF=0，BSD 沿用原检人检车组合。HD900-alg/pd/pd.cpp 对 Pallet OFF 跳过 PalletStateAnalysis，保持该帧目标、灯请求为空，也不打印推理成功日志。没有改变 FF CC 配置包格式。Sonix 同时过滤关检测后的残留目标、声音和自动灯请求；手动灯设置沿用原行为。guideline 仍按 Pallet 通道显示，不受检测、声音或触发输出开关影响。
- 更新四张 AI_PALLET_SETUP BMP 为 820×388，统一放在 (32,88)。追加 AI_PALLET_SLIDER / AI_PALLET_SLIDER_GREEN_HL / AI_PALLET_SLIDER_RED_HL 三张资源，沿用 System 原控件两端像素，仅缩短中段。同步 OSD2Image_Table、OSDdata_1.ini 和两份 OSDImage.dat：共 953 张资源，14,645,917 字节，调色板及其余原有 946 张资源未变。

六项翻译如下（长文字按行宽换行，文字内容不缩写）：

| 中文 | 英文 | 德语 | 法语 |
| --- | --- | --- | --- |
| 激光灯关闭延时 | Laser Off Delay | Ausschaltverzögerung des Lasers | Délai d’extinction du laser |
| 栈板报警范围倍率 | Pallet Alarm Range Multiplier | Multiplikator für den Paletten-Alarmbereich | Multiplicateur de la zone d’alarme des palettes |
| 光流检测帧间隔 | Optical Flow Detection Frame Interval | Frame-Intervall für die Erkennung des optischen Flusses | Intervalle d’images pour la détection du flux optique |
| 光流暂停时长 | Optical Flow Pause Duration | Pausendauer des optischen Flusses | Durée de pause du flux optique |
| 光流持续运行时长 | Optical Flow Continuous Run Duration | Dauer des kontinuierlichen Optical-Flow-Betriebs | Durée de fonctionnement continu du flux optique |
| 栈板检测灵敏度 | Pallet Detection Sensitivity | Empfindlichkeit der Palettenerkennung | Sensibilité de détection des palettes |

图像来源：使用内置 imagegen 编辑四张原标签图。提示要求为 text-localization，纯黑底白字、统一字号的 Arial/中文宋体，完整保留上表准确文字和重音，按指定换行输出标签图集，不增加控件、序号或装饰；另含 Pallet Detection / Détection des palettes / Palettenerkennung / 栈板检测。生成标签按原行距裁取、缩放并套用现有灰度调色板，最终保存到 `510PF_ConfigGen_v20170713a/SNCC7X_Demo/Common/AI_PALLET_SETUP{,_FR,_GEN,_CHN}.bmp`。Trigger Out / Alarm Sound 的生成文字未采用，最终使用 BSD 原标签。

验证：提取实际菜单绘制、按键、触摸、配置迁移、SEI 映射、声音和串口接收代码，使用工程真实头文件和模拟硬件接口，通过 5,624 组临时检查；实际 1126 检测分支通过 148 组通道/模式/推理失败/反复开关场景，G++ C++11 无告警。按实际绘制函数和最终 DAT 渲染四种语言，核对文字与控件不重叠、换焦点不擦字；检查 953 条资源、两份 DAT 一致性及 BSD 标签像素复用。UI、菜单、UART 完成 GCC 兼容语法检查，无新增诊断；KNL 修改前后均存在同一个 tKNL_GetPlayDestFld 声明/定义返回类型冲突，本次未修改该无关问题，修改的 SEI 分支已独立编译验证。临时测试与预览文件已清理，没有新增正式测试。现有 BIN 未重新生成；需配套编译、更新 Sonix 代码和 OSD 资源以及 1126 alg，未完成 ARMCC/1126 全量交叉编译及真机验证。

2026-09-22：图片资源缩减至 16,000 KB 以内

- 仅裁去四张 `AICONFIG_TITLE{,_FR,_GEN,_CHN}.bmp` 的外围纯黑区域，不缩放、不重绘文字。英法德从 1024×600 裁为 760×367，位置改为 (131,33)；中文裁为 760×376，位置改为 (131,24)。原屏幕上的所有文字与分隔线位置、字体和像素不变。AI 页先画 SUBMENU2 底图，再画标题及返回按钮，因此返回按钮也按原逻辑显示。
- 同步四张 BMP、OSDdata_1.ini、OSD2Image_Table 和两份 OSDImage.dat。资源总数及编号仍为 953，未删除任何语言或控件；其余 949 张资源的像素、尺寸、位置和保留字段不变。DAT 从 14,645,917 降至 **13,310,837 字节**，节省 **1,335,080 字节**。
- 使用当前已编译的程序重新打包根目录 `SNCC7XFW.bin`，从 **16,965,632 字节（16,568 KiB）**降至 **15,630,336 字节（15,264 KiB）**。保持原 4 KiB 资源对齐和 32 KiB 固件对齐，更新固件总长及 Profile 中的图片分配长度、Logo/音频地址，并同步根目录 Profile.dat。程序代码与 SDK_BUC/BIN/SNCC7XFW.bin 完全一致（仅完整固件的总长字段不同），Logo 和 aduexamp_flow.aud32 的三段报警声音逐字节保留。
- 最终根目录 SNCC7XFW.bin SHA-256：`E1E91EB2463D73DDA99DB9509A8791A62777D69DD0600CDB5DDB6E361DE0453B`。

验证：四张裁剪图补回黑边后与原 1024×600 像素逐字节相同；四种语言×触摸返回按钮显示/隐藏共 8 种页面合成结果一致；953 条 INI/DAT 记录、BMP 尺寸、资源地址、调色板、两份 DAT 一致性通过检查。完整 BIN 内嵌 DAT、Profile、Logo、音频及填充区域全部通过验证，并确认程序区域没有变化。临时脚本、备份和验证文件已删除；没有新增正式测试。本次只处理资源并重新打包，没有重新编译程序或上板验证；旧 FIRMWARE.bin 和带产品名的历史 BIN 未改写。

2026-09-22：Pallet 法德文字换行及滑条左移

- 仅替换 AI_PALLET_SETUP_GEN.bmp 第三、第四项为三行，AI_PALLET_SETUP_FR.bmp 第四项为两行；译文不变。其余标签（包括右侧 Trigger Out / Alarm Sound）像素保持原样，BMP 尺寸仍为 820×388。
- 四种语言共用的滑条 X 从 422 调为 402，选中框 X 从 400 调为 380，统一左移 20 像素。只修改 UI_BUCCU[WSVGA].h 的两个坐标宏，绘制、擦除及触摸换算沿用同一宏；同步 OSD2Image_Table、OSDdata_1.ini 和两份 OSDImage.dat。
- 校验 953 条资源的尺寸、地址与 INI 坐标；两份 DAT 一致，其余 948 张资源与调色板不变。核对指定文字行数为 3/3/2，四种语言文字距选中框至少 61 像素。资源包仍为 13,310,837 字节，无体积增长。整页预览通过；未新增正式测试。
- 本次未进行 ARMCC 编译或上板验证，也未重新打包 SNCC7XFW.bin；现有 BIN 仍为上一版 15,264 KiB。新布局需重新编译 Sonix 并配套新 DAT 打包。

图像使用内置 imagegen 编辑，生成结果仅裁取所需三处文字、匹配现有字号和灰度调色板，合入原 BMP。最终提示词如下：

```text
GEN:
Use case: text-localization. Edit target: the attached German embedded UI bitmap AI_PALLET_SETUP_GEN.bmp. Pure black background, white regular Arial-style sans-serif text. Change ONLY the third and fourth left-column parameter labels to exactly THREE lines each, without abbreviating, translating, or changing any characters. Third label verbatim line breaks: "Frame-Intervall für die\nErkennung des optischen\nFlusses". Fourth label verbatim line breaks: "Pausendauer\ndes optischen\nFlusses". Keep the same letter size and weight as the other left-column labels. Use tight but clear line spacing so each three-line group fits inside its existing 68px parameter row, vertically centered on the original label center. Keep left alignment at x=0; on the original 820x388 canvas the third row center is y=160 and fourth y=228. Preserve all other left labels and all right-column labels exactly, including single-line Trigger-Ausgang and Alarmton. Preserve the original 820:388 aspect ratio and layout, no buttons, no slider, no new text, no decoration. Output a flat bitmap UI text asset.

FR:
Use case: text-localization. Edit target: attached French embedded UI bitmap AI_PALLET_SETUP_FR.bmp. Pure black background, white regular Arial-style sans-serif text. Change ONLY the fourth left-column parameter label to exactly TWO lines with exact text and line breaks: "Durée de pause\ndu flux optique". Keep its letter size and weight consistent with all other left-column labels, left aligned at x=0, with the two lines vertically centered on the original fourth row center (y=228 on original 820x388 canvas). Preserve every other left label and every right label exactly, especially the single-line Sortie déclencheur and Alarme sonore. Keep original 820:388 aspect ratio and layout. No controls, sliders, new text, or decorations. Output a flat bitmap UI text asset.
```

2026-09-22：校准页临时照明与 Guideline 开关

- `UI_SendLaserorLedToCAM` 在取得摄像头命令锁后读取当前菜单状态，为正在校准的通道临时开启 Laser / LED。复用现有灯控任务及状态，不新增任务或持久化字段；无光流请求、Pallet Detection 关闭或配置尚未应答时也可校准照明。只处理支持 LAS 的在线摄像头，手动常开优先。
- 返回、菜单超时退出或切换校准通道后，旧校准页的照明请求自动撤销；没有仍有效的光流请求时关闭临时灯。待机、断电和升级状态不维持校准临时照明；不改写保存的 Laser / LED 设置。
- 按用户确认，主画面 Pallet 校准线也受本通道 `GuideLineEnable` 控制，Pallet 与 Guideline 均开启才显示；BSD 的原 Guideline 开关行为不变。此规则覆盖前文“Pallet 始终显示”的旧要求。校准页本身仍显示待调整的线，单/双/四画面的坐标缩放保持原实现。
- 验证：实际灯控发送、灯控任务处理及画线函数使用项目真实头文件和模拟硬件接口，通过 149 项灯控检查及 3,072 组通道/分屏/算法/开关组合（24,576 项画线断言）；修改前分别复现缺少校准开灯、Pallet 忽略 Guideline 开关。主 UI 完成 GCC 兼容语法检查，临时副本仅适配两处既有 ARMCC 数组初始化，保留同一处既有隐式声明警告，无新增诊断。未生成 BIN，未进行 ARMCC 全量编译或上板验证；临时验证文件已清理。

2026-09-22：Pallet 德文第三、第四项恢复两行

- `AI_PALLET_SETUP_GEN.bmp` 第三项恢复为 `Frame-Intervall für die` / `Erkennung des optischen Flusses`，第四项为 `Pausendauer des` / `optischen Flusses`，译文不变；两组文字仍分别在原行中心对齐，中间留白 25 像素。
- 使用内置 imagegen 编辑，仅取第三、第四项文字，按原尺寸缩放并匹配原灰度调色板，保存到 `510PF_ConfigGen_v20170713a/SNCC7X_Demo/Common/AI_PALLET_SETUP_GEN.bmp`；其他标签像素、820×388 尺寸和滑条位置不变。同步根目录和 ConfigGen 目录的 `OSDImage.dat`。
- 验证两项均为两行，其他 952 张资源、全部资源元数据和调色板逐字节不变；两份 DAT 哈希一致，仍为 13,310,837 字节，已核对最终图片。没有修改 C 代码，未重新编译、打包 BIN 或上板验证；临时验证文件已清理。

本次 imagegen 最终提示词：

```text
Use case: text-localization. Edit target: attached German embedded UI bitmap AI_PALLET_SETUP_GEN.bmp (820x388). Change ONLY the third and fourth parameter labels in the LEFT column from three lines back to exactly TWO lines each. Third label exact text and line breaks: "Frame-Intervall für die\nErkennung des optischen Flusses". Fourth label exact text and line breaks: "Pausendauer des\noptischen Flusses". Do not abbreviate or change the German text. Match the existing regular Arial-style white font with glyph height about 21px on the original 820px-wide canvas. Keep x=0 left alignment. Third two-line group centered at y=160, fourth two-line group centered at y=228; each group should occupy about 46px high with enough black space separating the two groups. Keep the third group within x=0..340 of the original canvas, at the same font size as other labels. Pure black background. Preserve original 820:388 layout and ALL other labels and pixels, including right column Paletten-erkennung, Trigger-Ausgang, Alarmton. No new text, controls, decoration, or borders. Output flat bitmap text asset.
```
