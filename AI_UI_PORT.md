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
| tFlowPauseDuration | 0 | 0–50 / 5 |
| tFlowRunDuration | 10 | 10–30 / 5 |
| tDectability | 52 | 0–99 / 1 |

Flash

每路 UI_CamStatus_t 的原预留区使用 8 字节，保存算法选择、6 个参数及版本标记，剩余预留空间从 163 调整为 155 字节，原结构大小和既有字段偏移不变。默认值与合法性处理分别接入 UI_ResetUIParameter / UI_CheckUIParameter，保存与加载沿用 UI_UpdateDevStatusInfo / UI_LoadDevStatusInfo。退出 SETUP 时保存。旧版本或非法新增配置只恢复该通道的新增字段，不清除配对信息。

资源与验证

- OSDdata_1.ini、OSD2Image_Table[WSVGA].h、四张 AI_PALLET_SETUP 图片、根目录和 ConfigGen 目录的 OSDImage.dat 已配套更新。四张旧加减图标已从 BMP、资源表、INI 和两份 DAT 中删除，后续资源索引已同步调整。资源数量为 944，DAT 为 13,470,216 字节；本次删除保留了其余 944 张资源的像素和非地址元数据。
- 六行滑条位置从 (508,88) 开始，尺寸 440×48；选中框从 (486,84) 开始，尺寸固定为 486×56。文字与滑条行距统一为 68，四语言文字图片为 440×388；文字 X 坐标由资源配置（默认 252、法文 112、德文 128、中文 262），绘制函数不再区分宽框或单独裁剪最后一行。
- 本次临时 C 验证通过：1,064 个按键边界组合、22,128 个点击/拖动组合、全部合法值的滑块中心点击、48 个四语言重绘/不透底组合，以及四通道入口、BSD 开关、Cursor 通道、菜单焦点和三页返回高亮。配置结构、默认值与合法性校验未因布局改动而改变。
- 本次 SUBSUBMENU 文件通过 GCC 语法检查，保留原有三处隐式声明告警；临时菜单验证程序使用实际菜单函数和 DAT 资源，通过 GCC 编译及运行。未完成 ARMCC/Keil 全量编译、实际 ConfigGen 生成、固件烧录或真机验证。
- AI_UI_PREVIEW.png 为根据实际 UI 绘制调用与 BMP 合成的布局预览，不是真机截图。

本轮没有移植 Pallet 算法通信或音频播放优化。新图片资源与代码需要配套重新编译打包，现有发布 BIN 未更新。
