**WSVGA 图片资源复用排查**

**当前结果：追加自动扫描/停车线开关、配对/删除按钮，并同步生成配置**

- 本轮共删除 **52 张**：自动扫描/停车线的 80×42 开关 36 张，配对/删除摄像头按钮 16 张（含德文）。开关复用 `AUTOSCAN_OFF/ON_CAM1_NOR/HL`；删除按钮复用对应摄像头的 `PAIRCAM*`，按删除行重新定位。停车线 AUTO 状态仍保留原图片，读取步长由 6 调整为 2；引导线页面也同步使用共享开关。
- 本轮节省 **217,792 字节（212.69 KiB）**，其中像素 216,960 字节、元数据 832 字节。两轮累计删除 **96 张**，根目录 DAT 从 **12,912,088 → 12,235,864 字节**，累计节省 **676,224 字节（660.38 KiB）**；现为 OSD1 36 张、OSD2 892 张，共 **928 张**。
- 当前资源包 SHA-256：`6EA28287B83B08953CEEDA800D9A3268158017686CC274D0F4AFA1D7A1EE5087`。
- `510PF_ConfigGen_v20170713a/OSDdata_1.ini` 已补删两轮共 96 条记录，包含上一轮的 44 条；所有保留记录连续重编号为 1～928，`filecount` 与 `Total_image_count` 均为 928。按原 table 的序号映射删除，保留记录的图片路径、尺寸、坐标、图层等内容不变。新 INI、table 与 DAT 顺序一致，928 条的尺寸/坐标/图层及 BMP 文件存在性均通过核对。
- 配置生成器目录中的旧 `OSDImage.dat` 尚未重新生成；后续用更新后的 `_1.ini` 生成后，需要将生成结果同步到根目录，固件合并脚本读取根目录 DAT。已有发布 BIN 未改写。
- 新增的重定位数值统一为 `SDK_BUC/SRC/UI/HS/UI_BUCCU[WSVGA].h` 中的页面宏，55 处绘制/擦除/刷新/触摸表达式已替换；原有 `DATE_TIME_YEAR_DIST_X/Y` 也从 `_SUBSUBMENU.h` 移到该公共头文件。未重定位的轴和宽高仍来自保留图片的 DAT 元数据，头文件注释已注明。
- 当前验证：本轮 52 张删除图片与复用图片逐字节相同；928 张保留图片的像素和非地址元数据不变；没有残留引用两轮删除的 96 个枚举；常量连续读取范围中的图片顺序未改变。从实际 C 代码提取的批量定位、普通/高亮切换、状态切换及触摸定位通过 **2026 项像素/坐标对比**。三个 C 文件的 GCC 兼容性语法检查通过，仍为原有的 5 个隐式声明告警；`git diff --check` 通过。临时验证文件已清理，没有新增正式测试文件。
- 本轮涉及：上述 INI、根目录 DAT、`OSD2Image_Table[WSVGA].h`、`UI_BUCCU[WSVGA].h`、`UI_BUCCU[WSVGA]_SUBSUBMENU.h` 及三个菜单 C 文件。未完成 ConfigGen 实际生成、ARMCC/Keil 全量编译或真机触摸验证。**新枚举代码与新 DAT 必须配套重新编译打包。**

**真机触摸回归范围与宏**

坐标保持原图片位置；摄像头滑条原有的左右 50、上下 10 像素触摸扩展范围也保持不变。

| 页面 | 公共头文件中的坐标宏 | 需要测试 |
| --- | --- | --- |
| 摄像头参数 | `CAMERA_SLIDER_Y`、`CAMERA_HL_Y`、`CAMERA_Y_STEP` | 五行滑条拖动、首尾位置、红/绿高亮切换、离开后重进 |
| 日期/时间 | `DATE_TIME_YEAR_DIST_X/Y` | 年月日时分秒六格的点击、数值修改及高亮切换 |
| 触发设置 | `TRIGGER_X/Y`、`TRIGGER_X_STEP/Y_STEP` | 五个延时框、五个优先级框及编辑返回；显示设置按钮入口 |
| 自动扫描 | `AUTOSCAN_SWITCH_X/Y`、`AUTOSCAN_DELAY_X/Y`、`AUTOSCAN_X_STEP/Y_STEP` | 六路开关、六个延时框、编辑返回、高亮切换；至少一路开启的原有限制 |
| 停车线/引导线 | `PARKINGLINE_SWITCH_X/Y`、`PARKINGLINE_X_STEP/Y_STEP` | 四路开关，停车线 OFF/ON/AUTO、引导线 OFF/ON，标定入口及返回后的显示 |
| 配对/删除摄像头 | `PAIRING_Y`、`PAIRING_Y_STEP` | 两行各四个按钮的点击、高亮、配对状态标记；默认及德文按钮资源 |

**第一轮实施记录：摄像头滑条、高亮框及日期/触发/扫描数字框（历史结果）**

- 摄像头保留 `CAMS_BRIGHTNESS_SLIDER`、`CAMS_BRIGHTNESS_GREEN_HL/RED_HL`，删除其余四行的 12 张重复图片；绘制、擦除和触摸均按行恢复 y 坐标。
- 数字框保留 `DATE_TIME_YEAR_NOR/HL`，删除触发延时/优先级的 20 张及自动扫描延时的 12 张重复图片；同步处理进入页面、普通/高亮切换、数值刷新、返回页面和触摸坐标。日期刷新路径残留的连续读取 12 张已改为读取一对按钮后定位。
- 根目录 `OSDImage.dat` 已从 **12,912,088** 降至 **12,453,656** 字节，减少 **458,432 字节（447.69 KiB）**，其中像素数据 457,728 字节、元数据 704 字节。OSD1 仍为 36 张，OSD2 从 988 降至 944 张，总计 **1024 → 980 张**。
- 第一轮资源包 SHA-256：`E6435E7AAF43DF1011F581049F23BCDF44B69FEE36794D1EC2442AE0248650D5`。
- 修改文件：根目录资源包，`SDK_BUC/SRC/UI/HS/OSD2Image_Table[WSVGA].h`，同目录的 `UI_BUCCU[WSVGA]_SUBMENU.c`、`UI_BUCCU[WSVGA]_SUBSUBMENU.c`、`UI_BUCCU[WSVGA]_SUBSUBSUBMENU.c`，以及 `SDK_BUC/SDK_BUC.uvproj`、`SDK_BUC/UTILITY/Merge.bat`。
- HDW297RS 的工程构建后命令新增第 7 个参数 `OSD`，使实际输出名 `SNCC7XFW` 正确进入 OSD 打包分支；该分支使用根目录的新 `OSDImage.dat`。旧 SYSTEM/OSD 及 ConfigGen 目录下的资源包未覆盖。
- 验证通过：44 张删除项与保留首张逐字节一致；980 张保留图片的像素、宽高、默认坐标和保留字段均未改变；新枚举和 DAT 顺序一致；从实际修改代码提取的绘制/触摸逻辑通过 160 项图片及坐标对比；常量批量读取未残留跨越已删除项的范围；`git diff --check` 通过。
- 对三个修改的 C 文件完成 GCC 兼容性语法检查，与原版对照仍为相同的 5 个已有隐式声明告警。仅在临时副本中为原有 `case` 标签后的声明补空语句以兼容 GCC，没有修改这些无关源码。未完成 ARMCC/Keil 全量编译、链接、实际固件打包或真机验证；已有发布 BIN 未改写。**新代码和新 DAT 必须配套重新编译打包，不能只给旧固件替换图片包。**

**以下为优化前排查快照**

下文资源编号、地址、源码行号及“尚未修改”的描述均对应优化前版本，供继续排查其余资源使用；当前实施结果以上述记录为准。

原排查以项目根目录 `OSDImage.dat` 为准：36 张 OSD1、988 张 OSD2，共 1024 张。按相同宽高和像素索引数据 SHA-256 分组后，对重复项逐字节复核，确认 60 组、178 份重复图片数据，可减少 1,192,248 字节，约 1.14 MiB，占该资源包的 9.23%。这是完全一致资源的统计，不包含近似图片或裁剪背景的潜在收益。

其中 OSD2 内部有 58 组，可减少 176 份数据、1,189,048 字节；另有 2 组 OSD1/OSD2 静音图标重复，可减少 3,200 字节。跨层资源只能共享像素数据，调用时仍须使用各层自己的资源编号。

**资源版本核对**

| 资源包 | OSD1 / OSD2 张数 | 文件字节数 | 结论 |
| --- | ---: | ---: | --- |
| 根目录 `OSDImage.dat` | 36 / 988 | 12,912,088 | 与当前 WSVGA 资源枚举数量一致；本次统计对象 |
| `510PF_ConfigGen_v20170713a/OSDImage.dat` | 36 / 986 | 12,901,336 | 另一版本，不能直接套用本清单的全部编号 |
| `SDK_BUC/SYSTEM/OSD/OSD_Image_WSVGA_0°.dat` | 11 / 812 | 7,573,286 | 与当前枚举不一致，不能按当前名称解释其编号 |

根目录资源包 SHA-256：`73DE1DC1D944AA381F40EE836E5B60D1D9AAAA8DDB550FC43804567198E31279`。

在 `FIRMWARE.bin`、`SNCC7XFW.bin`、`ST-HDW297RS-FCC-AI-R-00-V1.4-260623.bin` 中，均在 `0x20C000` 找到根目录资源包，完整 12,912,088 字节对比均一致。因此本清单也适用于这三份已有固件内的图片资源。

`SDK_BUC/UTILITY/Merge.bat:154` 仍拼接 SYSTEM/OSD 目录下的旧包，且 `SDK_BUC/SDK_BUC.uvproj:3651` 的 HDW297RS 构建后步骤调用该脚本。实际执行优化时必须同步确认最终打包输入，不能只修改根目录资源包后就认为重新编译会自动采用它。本次没有变更打包流程。

**优先复用的资源**

以下每种普通态、高亮态、开/关状态分别保留第一张，不把不同状态合并。

| 资源类别 | 保留第一组 | 可减少图片份数 | 可减少像素字节 |
| --- | --- | ---: | ---: |
| 摄像头滑条及红/绿高亮框 | `CAMS_BRIGHTNESS_SLIDER`、`CAMS_BRIGHTNESS_GREEN_HL/RED_HL` | 12 | 265,728 |
| 日期、触发延时/优先级、扫描延时的空白数字框 | `DATE_TIME_YEAR_NOR/HL` | 32 | 192,000 |
| 系统亮度、白天/夜间滑条及红/绿高亮框 | `SYSTEM_DIMMER_SLIDER/GREEN_HL/RED_HL` | 6 | 151,104 |
| 系统、录像、时间戳、AI 的 107×42 开关 | `SYSTEM_AUTO_OFF_NOR/HL`、`SYSTEM_AUTO_ON_NOR/HL` | 28 | 125,832 |
| 自动扫描、停车线的 80×42 开关 | `AUTOSCAN_OFF_CAM1_NOR/HL`、`AUTOSCAN_ON_CAM1_NOR/HL` | 36 | 120,960 |
| 配对与删除摄像头按钮，含德文 | 对应摄像头和状态的 `PAIRCAM*` | 16 | 96,000 |
| 摄像头镜像、翻转、激光、LED 开关 | `CAMS_H_MIRROR_OFF_NOR/HL`、`CAMS_H_MIRROR_ON_NOR/HL` | 12 | 53,928 |
| 停车线标定按钮 | `PARKINGLINE_CAM1_CAL_NOR/HL` | 6 | 36,000 |
| 停车线 AUTO 按钮 | `PARKINGLINE_CAM1_AUTO_NOR/HL` | 6 | 20,160 |
| 其余文字、圆形滑块、上下边线及跨层静音图标 | 见完整清单 | 24 | 130,536 |
| 合计 | | 178 | 1,192,248 |

**按第一张图片重新定位的具体坐标**

下表均为根目录 DAT 中的实际坐标，不能用资源头文件宏参数里的示意坐标代替。`UI.h:193` 的 `OSD2IMGPOOL` 宏只生成枚举，不使用其中的地址及 x/y；运行时位置来自 `tOSD_GetOsdImgInfor()` 读取的 DAT 记录。

| 用途 | 复用资源 | 目标位置规则 |
| --- | --- | --- |
| 摄像头五行滑条 | `CAMS_BRIGHTNESS_SLIDER` | `x=250; y=180+70*i; i=0..4`，大小 250×48 |
| 摄像头五行高亮框 | `CAMS_BRIGHTNESS_GREEN_HL/RED_HL` | `x=30; y=176+70*i; i=0..4`，大小 486×56 |
| 摄像头四行开关 | `CAMS_H_MIRROR_OFF/ON_NOR/HL` | `x=750; y=185+70*i; i=0..3`，大小 107×42 |
| 配对/删除按钮 | 同摄像头的 `PAIRCAM*` | `x=380+140*cam; cam=0..3`；配对 `y=215`，删除 `y=315`，大小 100×60 |
| 触发延时空白框 | `DATE_TIME_YEAR_NOR/HL` | `x=220+140*i; y=200; i=0..4`，大小 100×60 |
| 触发优先级空白框 | `DATE_TIME_YEAR_NOR/HL` | `x=220+140*i; y=280; i=0..4`，大小 100×60；不包括带数字的 `TRIGGER_PRIORITY1..5` |
| 自动扫描延时框 | `DATE_TIME_YEAR_NOR/HL` | `x=340+420*(i%2); y=185+95*(i/2); i=0..5` |
| 自动扫描开关 | `AUTOSCAN_OFF/ON_CAM1_NOR/HL` | `x=240+420*(i%2); y=195+95*(i/2); i=0..5` |
| 停车线标定按钮 | `PARKINGLINE_CAM1_CAL_NOR/HL` | `x=350+400*(i%2); y=205+140*(i/2); i=0..3` |
| 停车线开关/AUTO | 对应的第一套开关/AUTO | `x=250+400*(i%2); y=215+140*(i/2); i=0..3` |

上面的 `/` 运算按 C 的整数除法理解。完整清单保留了所有目标位置，包括没有规则排列的系统开关及多语言文字。

已有写法可以直接沿用：`UI_BUCCU[WSVGA]_SUBSUBMENU.c:542` 的 `UI_SystemDrawDateTimeYearButton()` 已用一套年按钮绘制年月日时分秒；AI 页面也已经根据 `AI_Config_Xpos/Ypos/Xstep/Ystep` 重复绘制同一套按钮，触摸函数使用相同的网格参数。

若采用“删除重复条目，只加载第一张”的方式，摄像头滑条的加载后定位可保持如下简单写法，绘制和触摸读取后都要应用同一规则：

```c
tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS_BRIGHTNESS_SLIDER, 1, &tOsdImgInfo);
tOsdImgInfo.uwXStart = 250;
tOsdImgInfo.uwYStart = 180 + 70*i;
```

**绘制、触摸及索引需要联动的位置**

| 模块 | 绘制/刷新入口 | 触摸入口与注意点 |
| --- | --- | --- |
| 配对/删除 | `UI_BUCCU[WSVGA]_SUBMENU.c:115`、`:697`；当前连续读取 16 张或通过 `+2*i` 取图 | 同文件 `:1975` 的 `UI_PairingSubTouchMenu()`；删除行须恢复 `y=315` |
| 摄像头参数 | `UI_BUCCU[WSVGA]_SUBSUBMENU.c:2237`、`:2252`、`:2272`；`UI_BUCCU[WSVGA]_SUBSUBSUBMENU.c:344`、`:407` | 后者 `:3554` 的 `UI_CameraSubSubSubTouchMenu()`；开关点击和 `:3663` 滑动都依赖 DAT 坐标；滑条还保留横向 ±50、纵向 ±10 的触摸容差 |
| 系统/日夜亮度 | `UI_BUCCU[WSVGA]_SUBMENU.c:1079`；`UI_BUCCU[WSVGA]_SUBSUBMENU.c:642`；`UI_BUCCU[WSVGA]_SUBSUBSUBMENU.c:846` | `UI_BUCCU[WSVGA]_SUBSUBMENU.c:3244`、`UI_BUCCU[WSVGA]_SUBSUBSUBMENU.c:3697`；包括滑块值与 x 坐标映射 |
| 触发、自动扫描、停车线 | `UI_BUCCU[WSVGA]_SUBMENU.c:1361`、`:1411`、`:1481`；`UI_BUCCU[WSVGA]_SUBSUBMENU.c:2000`、`:2057` | `UI_BUCCU[WSVGA]_SUBSUBMENU.c:3513`、`:3590`、`:3757`；触摸数组及按编号偏移的读取都要一起处理 |
| AI 开关 | `UI_BUCCU[WSVGA]_SUBMENU.c:1547`；`UI_BUCCU[WSVGA]_SUBSUBMENU.c:812` | 后者 `:3157`；保留实际网格位置，不能直接采用 `AI_OFF_NOR` 的默认 `(30,135)` |

以上文件均位于 `SDK_BUC/SRC/UI/HS/`。这些是已核对的主要入口；删除条目时还需按完整清单检查所有间接索引及批量读取，不能只搜索被删图片名称是否出现。

**两种落地方式**

1. 若目标主要是减少 Flash 占用，建议保留每个 16 字节 `OSD_IMG_INFO` 记录及原有编号、宽高、x/y，只把重复记录的 `ulAddrSft` 指向每组第一张保留的像素数据。重新排列数据区并更新所有偏移及必要包头字段后，资源包理论上由 12,912,088 降至 11,719,840 字节。这样绘制、批量读取和触摸仍使用原有位置，无需新增定位层。仅修改地址而不移除冗余数据区，不会缩小文件。此方案是由现有加载实现确认可行的建议，本次未生成或上机验证重打包文件。
2. 若还需要释放 ConfigGen 图片条目，按用户提出的方式保留第一张，并在各页面重新设置 x/y 与触摸坐标。全部精确重复条目合并后可从 1024 条降至 846 条，除像素外再省 2,848 字节元数据，资源包理论上减少 1,195,096 字节。代价是要同步调整枚举、批量加载、`+2*i`/`+4*i` 等索引运算，以及绘制、擦除、焦点刷新和触摸逻辑。`ConfigGen.INI:115` 当前配置 `iTotalImageCount=1024`，所以这条路线对释放条目也有价值。跨层 2 条不能直接拿另一层的枚举当别名，需单独处理。

仅在 C 代码中改为使用第一张而不重新生成资源包，不能减少 DAT 的空间；仅删除 `OSD2Image_Table[WSVGA].h` 中的行，也会造成枚举与 DAT 顺序不一致。建议若选择重新定位路线，先处理摄像头五行滑条和高亮框：范围集中，12 张重复数据共 265,728 字节。

判断依据：`COMMON_SRC/OSD/OSD.h:379` 定义图片元数据；`COMMON_SRC/OSD/OSD.c:1159` 按编号直接读取连续记录；`:919` 按 `ulAddrSft` 读取 `宽×高` 字节的像素索引；`:1066` 起的 `tOSD_Img2()` 使用传入的 x/y 绘制。

**不能直接合并的相似资源**

- `SYSTEM_AUTO_OFF_NOR` 与 `CAMS_H_MIRROR_OFF_NOR` 同为 107×42，但有 50 个像素的调色板值不同，不能算作完全一致；本清单保留为两个独立系列。
- `SYSTEM_DIMMER_GREEN_HL` 与 `CAMS_BRIGHTNESS_GREEN_HL` 同为 486×56，像素索引有 25,086 处不同，不能按尺寸直接合并。
- `PAIRCAM1NOR_ICON` 是 100×60，而 `SELCAM1NOR_ICON` 是 80×67，不能直接替代。
- `AI_TITLE/AI_TITLE_FR/AI_TITLE_GEN/AI_TITLE_CHN` 各为 1024×600，总占 2,457,600 字节；与第一张相比后三张分别有 17,976、16,794、20,218 个像素索引不同。可以作为后续“公共背景 + 各语言局部区域”的候选，但不属于本次 60 组精确重复。系统多语言大背景也属于需进一步拆图评估的候选；这部分收益未计入统计。
- 当前恰好相同的跨语言文字可以复用；后续若修改翻译，应允许其重新成为独立资源。

**验证范围**

- 已核对资源数量、所有记录的图片范围，以及 1024 张图片数据紧密覆盖文件数据区。
- 已完成 178 份重复数据逐字节校验；重复项目前使用不同地址，尚未共享数据。
- 对相同尺寸图片另按调色板值归一化并保留透明索引区别复查，未发现额外的同色重复组。
- 已核对三份现有固件内的完整资源包，均与本次统计对象一致。
- 本次不改运行代码，没有编译或上机触摸验证；后续实施后需检查普通/高亮状态、遥控焦点切换、滑动数值、返回刷新及各语言布局。

**完整精确重复清单**

每组保留当前资源顺序最先出现的一张。下列名称带层前缀；编号从 1 开始，0 是枚举占位项。位置是 DAT 默认位置，运行时另有动态定位的页面仍应沿用其动态布局。源码行号指向相应层的 `OSD1Image_Table[WSVGA].h` 或 `OSD2Image_Table[WSVGA].h`。

**01. OSD2IMG_CAMS_BRIGHTNESS_RED_HL：486×56，5 张保留 1 张，减少 108,864 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 355 | `OSD2IMG_CAMS_BRIGHTNESS_RED_HL` | 30 | 176 | `0x38D8B7` | 475 |
| 复用首张 | 357 | `OSD2IMG_CAMS_CONTRAST_RED_HL` | 30 | 246 | `0x39AD57` | 477 |
| 复用首张 | 359 | `OSD2IMG_CAMS_SATURATION_RED_HL` | 30 | 316 | `0x3A81F7` | 479 |
| 复用首张 | 361 | `OSD2IMG_CAMS_CHROME_RED_HL` | 30 | 386 | `0x3B5697` | 481 |
| 复用首张 | 363 | `OSD2IMG_CAMS_VOLUME_RED_HL` | 30 | 456 | `0x3C2B37` | 483 |

**02. OSD2IMG_CAMS_BRIGHTNESS_GREEN_HL：486×56，5 张保留 1 张，减少 108,864 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 354 | `OSD2IMG_CAMS_BRIGHTNESS_GREEN_HL` | 30 | 176 | `0x386E67` | 474 |
| 复用首张 | 356 | `OSD2IMG_CAMS_CONTRAST_GREEN_HL` | 30 | 246 | `0x394307` | 476 |
| 复用首张 | 358 | `OSD2IMG_CAMS_SATURATION_GREEN_HL` | 30 | 316 | `0x3A17A7` | 478 |
| 复用首张 | 360 | `OSD2IMG_CAMS_CHROME_GREEN_HL` | 30 | 386 | `0x3AEC47` | 480 |
| 复用首张 | 362 | `OSD2IMG_CAMS_VOLUME_GREEN_HL` | 30 | 456 | `0x3BC0E7` | 482 |

**03. OSD2IMG_DATE_TIME_YEAR_NOR：100×60，17 张保留 1 张，减少 96,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 402 | `OSD2IMG_DATE_TIME_YEAR_NOR` | 310 | 117 | `0x41A280` | 538 |
| 复用首张 | 450 | `OSD2IMG_TRIGGER_DELAY_CAM1_NOR` | 220 | 200 | `0x460087` | 609 |
| 复用首张 | 452 | `OSD2IMG_TRIGGER_DELAY_CAM2_NOR` | 360 | 200 | `0x462F67` | 611 |
| 复用首张 | 454 | `OSD2IMG_TRIGGER_DELAY_CAM3_NOR` | 500 | 200 | `0x465E47` | 613 |
| 复用首张 | 456 | `OSD2IMG_TRIGGER_DELAY_CAM4_NOR` | 640 | 200 | `0x468D27` | 615 |
| 复用首张 | 458 | `OSD2IMG_TRIGGER_DELAY_CAMSPLIT_NOR` | 780 | 200 | `0x46BC07` | 617 |
| 复用首张 | 460 | `OSD2IMG_TRIGGER_PRIORITY_CAM1_NOR` | 220 | 280 | `0x46EAE7` | 620 |
| 复用首张 | 462 | `OSD2IMG_TRIGGER_PRIORITY_CAM2_NOR` | 360 | 280 | `0x4719C7` | 622 |
| 复用首张 | 464 | `OSD2IMG_TRIGGER_PRIORITY_CAM3_NOR` | 500 | 280 | `0x4748A7` | 624 |
| 复用首张 | 466 | `OSD2IMG_TRIGGER_PRIORITY_CAM4_NOR` | 640 | 280 | `0x477787` | 626 |
| 复用首张 | 468 | `OSD2IMG_TRIGGER_PRIORITY_CAMSPLIT_NOR` | 780 | 280 | `0x47A667` | 628 |
| 复用首张 | 489 | `OSD2IMG_AUTOSCAN_DELAY_CAM1_NOR` | 340 | 185 | `0x4ACC9B` | 656 |
| 复用首张 | 491 | `OSD2IMG_AUTOSCAN_DELAY_CAM2_NOR` | 760 | 185 | `0x4AFB7B` | 658 |
| 复用首张 | 493 | `OSD2IMG_AUTOSCAN_DELAY_CAM3_NOR` | 340 | 280 | `0x4B2A5B` | 660 |
| 复用首张 | 495 | `OSD2IMG_AUTOSCAN_DELAY_CAM4_NOR` | 760 | 280 | `0x4B593B` | 662 |
| 复用首张 | 497 | `OSD2IMG_AUTOSCAN_DELAY_DUAL_NOR` | 340 | 375 | `0x4B881B` | 664 |
| 复用首张 | 499 | `OSD2IMG_AUTOSCAN_DELAY_QUAD_NOR` | 760 | 375 | `0x4BB6FB` | 666 |

**04. OSD2IMG_DATE_TIME_YEAR_HL：100×60，17 张保留 1 张，减少 96,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 403 | `OSD2IMG_DATE_TIME_YEAR_HL` | 310 | 117 | `0x41B9F0` | 539 |
| 复用首张 | 451 | `OSD2IMG_TRIGGER_DELAY_CAM1_HL` | 220 | 200 | `0x4617F7` | 610 |
| 复用首张 | 453 | `OSD2IMG_TRIGGER_DELAY_CAM2_HL` | 360 | 200 | `0x4646D7` | 612 |
| 复用首张 | 455 | `OSD2IMG_TRIGGER_DELAY_CAM3_HL` | 500 | 200 | `0x4675B7` | 614 |
| 复用首张 | 457 | `OSD2IMG_TRIGGER_DELAY_CAM4_HL` | 640 | 200 | `0x46A497` | 616 |
| 复用首张 | 459 | `OSD2IMG_TRIGGER_DELAY_CAMSPLIT_HL` | 780 | 200 | `0x46D377` | 618 |
| 复用首张 | 461 | `OSD2IMG_TRIGGER_PRIORITY_CAM1_HL` | 220 | 280 | `0x470257` | 621 |
| 复用首张 | 463 | `OSD2IMG_TRIGGER_PRIORITY_CAM2_HL` | 360 | 280 | `0x473137` | 623 |
| 复用首张 | 465 | `OSD2IMG_TRIGGER_PRIORITY_CAM3_HL` | 500 | 280 | `0x476017` | 625 |
| 复用首张 | 467 | `OSD2IMG_TRIGGER_PRIORITY_CAM4_HL` | 640 | 280 | `0x478EF7` | 627 |
| 复用首张 | 469 | `OSD2IMG_TRIGGER_PRIORITY_CAMSPLIT_HL` | 780 | 280 | `0x47BDD7` | 629 |
| 复用首张 | 490 | `OSD2IMG_AUTOSCAN_DELAY_CAM1_HL` | 340 | 185 | `0x4AE40B` | 657 |
| 复用首张 | 492 | `OSD2IMG_AUTOSCAN_DELAY_CAM2_HL` | 760 | 185 | `0x4B12EB` | 659 |
| 复用首张 | 494 | `OSD2IMG_AUTOSCAN_DELAY_CAM3_HL` | 340 | 280 | `0x4B41CB` | 661 |
| 复用首张 | 496 | `OSD2IMG_AUTOSCAN_DELAY_CAM4_HL` | 760 | 280 | `0x4B70AB` | 663 |
| 复用首张 | 498 | `OSD2IMG_AUTOSCAN_DELAY_DUAL_HL` | 340 | 375 | `0x4B9F8B` | 665 |
| 复用首张 | 500 | `OSD2IMG_AUTOSCAN_DELAY_QUAD_HL` | 760 | 375 | `0x4BCE6B` | 667 |

**05. OSD2IMG_SYSTEM_DIMMER_RED_HL：486×56，3 张保留 1 张，减少 54,432 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 247 | `OSD2IMG_SYSTEM_DIMMER_RED_HL` | 290 | 160 | `0x2BA57B` | 327 |
| 复用首张 | 283 | `OSD2IMG_SYSTEM_DAY_RED_HL` | 310 | 200 | `0x2F3B43` | 379 |
| 复用首张 | 285 | `OSD2IMG_SYSTEM_NIGHT_RED_HL` | 310 | 300 | `0x300FE3` | 382 |

**06. OSD2IMG_SYSTEM_DIMMER_GREEN_HL：486×56，3 张保留 1 张，减少 54,432 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 246 | `OSD2IMG_SYSTEM_DIMMER_GREEN_HL` | 290 | 160 | `0x2B3B2B` | 326 |
| 复用首张 | 282 | `OSD2IMG_SYSTEM_DAY_GREEN_HL` | 310 | 200 | `0x2ED0F3` | 378 |
| 复用首张 | 284 | `OSD2IMG_SYSTEM_NIGHT_GREEN_HL` | 310 | 300 | `0x2FA593` | 381 |

**07. OSD2IMG_CAMS_BRIGHTNESS_SLIDER：250×48，5 张保留 1 张，减少 48,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 330 | `OSD2IMG_CAMS_BRIGHTNESS_SLIDER` | 250 | 180 | `0x365027` | 444 |
| 复用首张 | 331 | `OSD2IMG_CAMS_CONTRAST_SLIDER` | 250 | 250 | `0x367F07` | 445 |
| 复用首张 | 332 | `OSD2IMG_CAMS_SATURATION_SLIDER` | 250 | 320 | `0x36ADE7` | 446 |
| 复用首张 | 333 | `OSD2IMG_CAMS_CHROME_SLIDER` | 250 | 390 | `0x36DCC7` | 447 |
| 复用首张 | 334 | `OSD2IMG_CAMS_VOLUME_SLIDER` | 250 | 460 | `0x370BA7` | 448 |

**08. OSD2IMG_SYSTEM_DIMMER_SLIDER：440×48，3 张保留 1 张，减少 42,240 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 242 | `OSD2IMG_SYSTEM_DIMMER_SLIDER` | 312 | 164 | `0x2ACDAB` | 322 |
| 复用首张 | 280 | `OSD2IMG_SYSTEM_DAY_SLIDER` | 330 | 204 | `0x2E2BF3` | 375 |
| 复用首张 | 281 | `OSD2IMG_SYSTEM_NIGHT_SLIDER` | 330 | 304 | `0x2E7E73` | 376 |

**09. OSD2IMG_SYSTEM_AUTO_ON_NOR：107×42，8 张保留 1 张，减少 31,458 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 250 | `OSD2IMG_SYSTEM_AUTO_ON_NOR` | 400 | 235 | `0x2C32E7` | 331 |
| 复用首张 | 256 | `OSD2IMG_SYSTEM_MENULOCK_ON_NOR` | 300 | 305 | `0x2C998B` | 339 |
| 复用首张 | 266 | `OSD2IMG_SYSTEM_SD_SWITCH_ON_NOR` | 760 | 305 | `0x2D4107` | 353 |
| 复用首张 | 270 | `OSD2IMG_SYSTEM_HIDE_CHANNEL_NAME_ON_NOR` | 760 | 375 | `0x2D873F` | 358 |
| 复用首张 | 274 | `OSD2IMG_SYSTEM_BUZZER_ON_NOR` | 760 | 438 | `0x2DCD77` | 363 |
| 复用首张 | 385 | `OSD2IMG_POWERON_REC_ON_NOR` | 380 | 343 | `0x3FA0BE` | 515 |
| 复用首张 | 389 | `OSD2IMG_TIME_STAMP_ON_NOR` | 380 | 418 | `0x3FE6F6` | 520 |
| 复用首张 | 972 | `OSD2IMG_AI_ON_NOR` | 30 | 135 | `0xC37432` | 1368 |

**10. OSD2IMG_SYSTEM_AUTO_ON_HL：107×42，8 张保留 1 张，减少 31,458 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 251 | `OSD2IMG_SYSTEM_AUTO_ON_HL` | 400 | 235 | `0x2C4475` | 332 |
| 复用首张 | 257 | `OSD2IMG_SYSTEM_MENULOCK_ON_HL` | 300 | 305 | `0x2CAB19` | 340 |
| 复用首张 | 267 | `OSD2IMG_SYSTEM_SD_SWITCH_ON_HL` | 760 | 305 | `0x2D5295` | 354 |
| 复用首张 | 271 | `OSD2IMG_SYSTEM_HIDE_CHANNEL_NAME_ON_HL` | 760 | 375 | `0x2D98CD` | 359 |
| 复用首张 | 275 | `OSD2IMG_SYSTEM_BUZZER_ON_HL` | 760 | 438 | `0x2DDF05` | 364 |
| 复用首张 | 386 | `OSD2IMG_POWERON_REC_ON_HL` | 380 | 343 | `0x3FB24C` | 516 |
| 复用首张 | 390 | `OSD2IMG_TIME_STAMP_ON_HL` | 380 | 418 | `0x3FF884` | 521 |
| 复用首张 | 973 | `OSD2IMG_AI_ON_HL` | 30 | 135 | `0xC385C0` | 1369 |

**11. OSD2IMG_SYSTEM_AUTO_OFF_NOR：107×42，8 张保留 1 张，减少 31,458 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 248 | `OSD2IMG_SYSTEM_AUTO_OFF_NOR` | 400 | 235 | `0x2C0FCB` | 329 |
| 复用首张 | 254 | `OSD2IMG_SYSTEM_MENULOCK_OFF_NOR` | 300 | 305 | `0x2C766F` | 337 |
| 复用首张 | 264 | `OSD2IMG_SYSTEM_SD_SWITCH_OFF_NOR` | 760 | 305 | `0x2D1DEB` | 351 |
| 复用首张 | 268 | `OSD2IMG_SYSTEM_HIDE_CHANNEL_NAME_OFF_NOR` | 760 | 375 | `0x2D6423` | 356 |
| 复用首张 | 272 | `OSD2IMG_SYSTEM_BUZZER_OFF_NOR` | 760 | 438 | `0x2DAA5B` | 361 |
| 复用首张 | 383 | `OSD2IMG_POWERON_REC_OFF_NOR` | 380 | 343 | `0x3F7DA2` | 513 |
| 复用首张 | 387 | `OSD2IMG_TIME_STAMP_OFF_NOR` | 380 | 418 | `0x3FC3DA` | 518 |
| 复用首张 | 970 | `OSD2IMG_AI_OFF_NOR` | 30 | 135 | `0xC35116` | 1366 |

**12. OSD2IMG_SYSTEM_AUTO_OFF_HL：107×42，8 张保留 1 张，减少 31,458 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 249 | `OSD2IMG_SYSTEM_AUTO_OFF_HL` | 400 | 235 | `0x2C2159` | 330 |
| 复用首张 | 255 | `OSD2IMG_SYSTEM_MENULOCK_OFF_HL` | 300 | 305 | `0x2C87FD` | 338 |
| 复用首张 | 265 | `OSD2IMG_SYSTEM_SD_SWITCH_OFF_HL` | 760 | 305 | `0x2D2F79` | 352 |
| 复用首张 | 269 | `OSD2IMG_SYSTEM_HIDE_CHANNEL_NAME_OFF_HL` | 760 | 375 | `0x2D75B1` | 357 |
| 复用首张 | 273 | `OSD2IMG_SYSTEM_BUZZER_OFF_HL` | 760 | 438 | `0x2DBBE9` | 362 |
| 复用首张 | 384 | `OSD2IMG_POWERON_REC_OFF_HL` | 380 | 343 | `0x3F8F30` | 514 |
| 复用首张 | 388 | `OSD2IMG_TIME_STAMP_OFF_HL` | 380 | 418 | `0x3FD568` | 519 |
| 复用首张 | 971 | `OSD2IMG_AI_OFF_HL` | 30 | 135 | `0xC362A4` | 1367 |

**13. OSD2IMG_AUTOSCAN_ON_CAM1_HL：80×42，10 张保留 1 张，减少 30,240 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 504 | `OSD2IMG_AUTOSCAN_ON_CAM1_HL` | 240 | 195 | `0x4C0D3B` | 672 |
| 复用首张 | 508 | `OSD2IMG_AUTOSCAN_ON_CAM2_HL` | 660 | 195 | `0x4C41BB` | 677 |
| 复用首张 | 512 | `OSD2IMG_AUTOSCAN_ON_CAM3_HL` | 240 | 290 | `0x4C763B` | 682 |
| 复用首张 | 516 | `OSD2IMG_AUTOSCAN_ON_CAM4_HL` | 660 | 290 | `0x4CAABB` | 687 |
| 复用首张 | 520 | `OSD2IMG_AUTOSCAN_ON_DUAL_HL` | 240 | 385 | `0x4CDF3B` | 692 |
| 复用首张 | 524 | `OSD2IMG_AUTOSCAN_ON_QUAD_HL` | 660 | 385 | `0x4D13BB` | 697 |
| 复用首张 | 548 | `OSD2IMG_PARKINGLINE_CAM1_EN_HL` | 250 | 215 | `0x50FD39` | 759 |
| 复用首张 | 554 | `OSD2IMG_PARKINGLINE_CAM2_EN_HL` | 650 | 215 | `0x514BF9` | 766 |
| 复用首张 | 560 | `OSD2IMG_PARKINGLINE_CAM3_EN_HL` | 250 | 355 | `0x519AB9` | 773 |
| 复用首张 | 566 | `OSD2IMG_PARKINGLINE_CAM4_EN_HL` | 650 | 355 | `0x51E979` | 780 |

**14. OSD2IMG_AUTOSCAN_ON_CAM1_NOR：80×42，10 张保留 1 张，减少 30,240 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 503 | `OSD2IMG_AUTOSCAN_ON_CAM1_NOR` | 240 | 195 | `0x4C001B` | 671 |
| 复用首张 | 507 | `OSD2IMG_AUTOSCAN_ON_CAM2_NOR` | 660 | 195 | `0x4C349B` | 676 |
| 复用首张 | 511 | `OSD2IMG_AUTOSCAN_ON_CAM3_NOR` | 240 | 290 | `0x4C691B` | 681 |
| 复用首张 | 515 | `OSD2IMG_AUTOSCAN_ON_CAM4_NOR` | 660 | 290 | `0x4C9D9B` | 686 |
| 复用首张 | 519 | `OSD2IMG_AUTOSCAN_ON_DUAL_NOR` | 240 | 385 | `0x4CD21B` | 691 |
| 复用首张 | 523 | `OSD2IMG_AUTOSCAN_ON_QUAD_NOR` | 660 | 385 | `0x4D069B` | 696 |
| 复用首张 | 547 | `OSD2IMG_PARKINGLINE_CAM1_EN_NOR` | 250 | 215 | `0x50F019` | 758 |
| 复用首张 | 553 | `OSD2IMG_PARKINGLINE_CAM2_EN_NOR` | 650 | 215 | `0x513ED9` | 765 |
| 复用首张 | 559 | `OSD2IMG_PARKINGLINE_CAM3_EN_NOR` | 250 | 355 | `0x518D99` | 772 |
| 复用首张 | 565 | `OSD2IMG_PARKINGLINE_CAM4_EN_NOR` | 650 | 355 | `0x51DC59` | 779 |

**15. OSD2IMG_AUTOSCAN_OFF_CAM1_NOR：80×42，10 张保留 1 张，减少 30,240 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 501 | `OSD2IMG_AUTOSCAN_OFF_CAM1_NOR` | 240 | 195 | `0x4BE5DB` | 669 |
| 复用首张 | 505 | `OSD2IMG_AUTOSCAN_OFF_CAM2_NOR` | 660 | 195 | `0x4C1A5B` | 674 |
| 复用首张 | 509 | `OSD2IMG_AUTOSCAN_OFF_CAM3_NOR` | 240 | 290 | `0x4C4EDB` | 679 |
| 复用首张 | 513 | `OSD2IMG_AUTOSCAN_OFF_CAM4_NOR` | 660 | 290 | `0x4C835B` | 684 |
| 复用首张 | 517 | `OSD2IMG_AUTOSCAN_OFF_DUAL_NOR` | 240 | 385 | `0x4CB7DB` | 689 |
| 复用首张 | 521 | `OSD2IMG_AUTOSCAN_OFF_QUAD_NOR` | 660 | 385 | `0x4CEC5B` | 694 |
| 复用首张 | 545 | `OSD2IMG_PARKINGLINE_CAM1_DIS_NOR` | 250 | 215 | `0x50D5D9` | 756 |
| 复用首张 | 551 | `OSD2IMG_PARKINGLINE_CAM2_DIS_NOR` | 650 | 215 | `0x512499` | 763 |
| 复用首张 | 557 | `OSD2IMG_PARKINGLINE_CAM3_DIS_NOR` | 250 | 355 | `0x517359` | 770 |
| 复用首张 | 563 | `OSD2IMG_PARKINGLINE_CAM4_DIS_NOR` | 650 | 355 | `0x51C219` | 777 |

**16. OSD2IMG_AUTOSCAN_OFF_CAM1_HL：80×42，10 张保留 1 张，减少 30,240 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 502 | `OSD2IMG_AUTOSCAN_OFF_CAM1_HL` | 240 | 195 | `0x4BF2FB` | 670 |
| 复用首张 | 506 | `OSD2IMG_AUTOSCAN_OFF_CAM2_HL` | 660 | 195 | `0x4C277B` | 675 |
| 复用首张 | 510 | `OSD2IMG_AUTOSCAN_OFF_CAM3_HL` | 240 | 290 | `0x4C5BFB` | 680 |
| 复用首张 | 514 | `OSD2IMG_AUTOSCAN_OFF_CAM4_HL` | 660 | 290 | `0x4C907B` | 685 |
| 复用首张 | 518 | `OSD2IMG_AUTOSCAN_OFF_DUAL_HL` | 240 | 385 | `0x4CC4FB` | 690 |
| 复用首张 | 522 | `OSD2IMG_AUTOSCAN_OFF_QUAD_HL` | 660 | 385 | `0x4CF97B` | 695 |
| 复用首张 | 546 | `OSD2IMG_PARKINGLINE_CAM1_DIS_HL` | 250 | 215 | `0x50E2F9` | 757 |
| 复用首张 | 552 | `OSD2IMG_PARKINGLINE_CAM2_DIS_HL` | 650 | 215 | `0x5131B9` | 764 |
| 复用首张 | 558 | `OSD2IMG_PARKINGLINE_CAM3_DIS_HL` | 250 | 355 | `0x518079` | 771 |
| 复用首张 | 564 | `OSD2IMG_PARKINGLINE_CAM4_DIS_HL` | 650 | 355 | `0x51CF39` | 778 |

**17. OSD2IMG_PARKINGLINE_CAM1_CAL_HL：100×60，4 张保留 1 张，减少 18,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 538 | `OSD2IMG_PARKINGLINE_CAM1_CAL_HL` | 350 | 205 | `0x5031C9` | 748 |
| 复用首张 | 540 | `OSD2IMG_PARKINGLINE_CAM2_CAL_HL` | 750 | 205 | `0x5060A9` | 750 |
| 复用首张 | 542 | `OSD2IMG_PARKINGLINE_CAM3_CAL_HL` | 350 | 345 | `0x508F89` | 752 |
| 复用首张 | 544 | `OSD2IMG_PARKINGLINE_CAM4_CAL_HL` | 750 | 345 | `0x50BE69` | 754 |

**18. OSD2IMG_PARKINGLINE_CAM1_CAL_NOR：100×60，4 张保留 1 张，减少 18,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 537 | `OSD2IMG_PARKINGLINE_CAM1_CAL_NOR` | 350 | 205 | `0x501A59` | 747 |
| 复用首张 | 539 | `OSD2IMG_PARKINGLINE_CAM2_CAL_NOR` | 750 | 205 | `0x504939` | 749 |
| 复用首张 | 541 | `OSD2IMG_PARKINGLINE_CAM3_CAL_NOR` | 350 | 345 | `0x507819` | 751 |
| 复用首张 | 543 | `OSD2IMG_PARKINGLINE_CAM4_CAL_NOR` | 750 | 345 | `0x50A6F9` | 753 |

**19. OSD2IMG_QUADVIEW_SETTING_WORD：150×50，3 张保留 1 张，减少 15,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 227 | `OSD2IMG_QUADVIEW_SETTING_WORD` | 132 | 325 | `0x233619` | 290 |
| 复用首张 | 754 | `OSD2IMG_QUADVIEW_SETTING_WORD_GER` | 132 | 325 | `0x5F22C8` | 997 |
| 复用首张 | 856 | `OSD2IMG_QUADVIEW_SETTING_WORD_FR` | 127 | 325 | `0x7905B6` | 1154 |

**20. OSD2IMG_DUALVIEW_SETTING_WORD：150×50，3 张保留 1 张，减少 15,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 226 | `OSD2IMG_DUALVIEW_SETTING_WORD` | 737 | 150 | `0x2318CD` | 289 |
| 复用首张 | 753 | `OSD2IMG_DUALVIEW_SETTING_WORD_GER` | 737 | 150 | `0x5F057C` | 996 |
| 复用首张 | 855 | `OSD2IMG_DUALVIEW_SETTING_WORD_FR` | 734 | 150 | `0x78E86A` | 1153 |

**21. OSD2IMG_CAMS_H_MIRROR_OFF_HL：107×42，4 张保留 1 张，减少 13,482 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 339 | `OSD2IMG_CAMS_H_MIRROR_OFF_HL` | 750 | 185 | `0x376715` | 455 |
| 复用首张 | 343 | `OSD2IMG_CAMS_V_MIRROR_OFF_HL` | 750 | 255 | `0x37AD4D` | 460 |
| 复用首张 | 347 | `OSD2IMG_CAMS_LASER_OFF_HL` | 750 | 325 | `0x37F385` | 465 |
| 复用首张 | 351 | `OSD2IMG_CAMS_LED_OFF_HL` | 750 | 395 | `0x3839BD` | 470 |

**22. OSD2IMG_CAMS_H_MIRROR_ON_NOR：107×42，4 张保留 1 张，减少 13,482 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 340 | `OSD2IMG_CAMS_H_MIRROR_ON_NOR` | 750 | 185 | `0x3778A3` | 456 |
| 复用首张 | 344 | `OSD2IMG_CAMS_V_MIRROR_ON_NOR` | 750 | 255 | `0x37BEDB` | 461 |
| 复用首张 | 348 | `OSD2IMG_CAMS_LASER_ON_NOR` | 750 | 325 | `0x380513` | 466 |
| 复用首张 | 352 | `OSD2IMG_CAMS_LED_ON_NOR` | 750 | 395 | `0x384B4B` | 471 |

**23. OSD2IMG_CAMS_H_MIRROR_ON_HL：107×42，4 张保留 1 张，减少 13,482 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 341 | `OSD2IMG_CAMS_H_MIRROR_ON_HL` | 750 | 185 | `0x378A31` | 457 |
| 复用首张 | 345 | `OSD2IMG_CAMS_V_MIRROR_ON_HL` | 750 | 255 | `0x37D069` | 462 |
| 复用首张 | 349 | `OSD2IMG_CAMS_LASER_ON_HL` | 750 | 325 | `0x3816A1` | 467 |
| 复用首张 | 353 | `OSD2IMG_CAMS_LED_ON_HL` | 750 | 395 | `0x385CD9` | 472 |

**24. OSD2IMG_CAMS_H_MIRROR_OFF_NOR：107×42，4 张保留 1 张，减少 13,482 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 338 | `OSD2IMG_CAMS_H_MIRROR_OFF_NOR` | 750 | 185 | `0x375587` | 454 |
| 复用首张 | 342 | `OSD2IMG_CAMS_V_MIRROR_OFF_NOR` | 750 | 255 | `0x379BBF` | 459 |
| 复用首张 | 346 | `OSD2IMG_CAMS_LASER_OFF_NOR` | 750 | 325 | `0x37E1F7` | 464 |
| 复用首张 | 350 | `OSD2IMG_CAMS_LED_OFF_NOR` | 750 | 395 | `0x38282F` | 469 |

**25. OSD2IMG_TRIGGER_DISPLAY_WORD：120×100，2 张保留 1 张，减少 12,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 444 | `OSD2IMG_TRIGGER_DISPLAY_WORD` | 90 | 350 | `0x45A51F` | 602 |
| 复用首张 | 819 | `OSD2IMG_TRIGGER_DISPLAY_WORD_GER` | 90 | 350 | `0x6EC068` | 1092 |

**26. OSD2IMG_PARKINGLINE_CAM1_AUTO_NOR：80×42，4 张保留 1 张，减少 10,080 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 549 | `OSD2IMG_PARKINGLINE_CAM1_AUTO_NOR` | 250 | 215 | `0x510A59` | 760 |
| 复用首张 | 555 | `OSD2IMG_PARKINGLINE_CAM2_AUTO_NOR` | 650 | 215 | `0x515919` | 767 |
| 复用首张 | 561 | `OSD2IMG_PARKINGLINE_CAM3_AUTO_NOR` | 250 | 355 | `0x51A7D9` | 774 |
| 复用首张 | 567 | `OSD2IMG_PARKINGLINE_CAM4_AUTO_NOR` | 650 | 355 | `0x51F699` | 781 |

**27. OSD2IMG_PARKINGLINE_CAM1_AUTO_HL：80×42，4 张保留 1 张，减少 10,080 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 550 | `OSD2IMG_PARKINGLINE_CAM1_AUTO_HL` | 250 | 215 | `0x511779` | 761 |
| 复用首张 | 556 | `OSD2IMG_PARKINGLINE_CAM2_AUTO_HL` | 650 | 215 | `0x516639` | 768 |
| 复用首张 | 562 | `OSD2IMG_PARKINGLINE_CAM3_AUTO_HL` | 250 | 355 | `0x51B4F9` | 775 |
| 复用首张 | 568 | `OSD2IMG_PARKINGLINE_CAM4_AUTO_HL` | 650 | 355 | `0x5203B9` | 782 |

**28. OSD2IMG_AUTOSCAN_CAM2_WORD_GER：112×70，2 张保留 1 张，减少 7,840 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 827 | `OSD2IMG_AUTOSCAN_CAM2_WORD_GER` | 540 | 180 | `0x7069AC` | 1105 |
| 复用首张 | 836 | `OSD2IMG_PARKINGLINE_CAM2_WD_GER` | 500 | 200 | `0x7244FD` | 1119 |

**29. OSD2IMG_AUTOSCAN_CAM3_WORD_GER：112×70，2 张保留 1 张，减少 7,840 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 828 | `OSD2IMG_AUTOSCAN_CAM3_WORD_GER` | 120 | 275 | `0x70884C` | 1106 |
| 复用首张 | 837 | `OSD2IMG_PARKINGLINE_CAM3_WD_GER` | 100 | 340 | `0x72639D` | 1120 |

**30. OSD2IMG_AUTOSCAN_CAM1_WORD_GER：112×70，2 张保留 1 张，减少 7,840 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 826 | `OSD2IMG_AUTOSCAN_CAM1_WORD_GER` | 120 | 180 | `0x704B0C` | 1104 |
| 复用首张 | 835 | `OSD2IMG_PARKINGLINE_CAM1_WD_GER` | 100 | 200 | `0x72265D` | 1118 |

**31. OSD2IMG_AUTOSCAN_QUAD_WORD：112×70，2 张保留 1 张，减少 7,840 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 488 | `OSD2IMG_AUTOSCAN_QUAD_WORD` | 540 | 370 | `0x4AADFB` | 654 |
| 复用首张 | 831 | `OSD2IMG_AUTOSCAN_QUAD_WORD_GER` | 540 | 370 | `0x70E42C` | 1109 |

**32. OSD2IMG_AUTOSCAN_DUAL_WORD：112×70，2 张保留 1 张，减少 7,840 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 487 | `OSD2IMG_AUTOSCAN_DUAL_WORD` | 120 | 370 | `0x4A8F5B` | 653 |
| 复用首张 | 830 | `OSD2IMG_AUTOSCAN_DUAL_WORD_GER` | 120 | 370 | `0x70C58C` | 1108 |

**33. OSD2IMG_AUTOSCAN_CAM4_WORD_GER：112×70，2 张保留 1 张，减少 7,840 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 829 | `OSD2IMG_AUTOSCAN_CAM4_WORD_GER` | 540 | 275 | `0x70A6EC` | 1107 |
| 复用首张 | 838 | `OSD2IMG_PARKINGLINE_CAM4_WD_GER` | 500 | 340 | `0x72823D` | 1121 |

**34. OSD2IMG_AUTOSCAN_SETTING_WORD：150×50，2 张保留 1 张，减少 7,500 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 229 | `OSD2IMG_AUTOSCAN_SETTING_WORD` | 537 | 325 | `0x2370B1` | 292 |
| 复用首张 | 756 | `OSD2IMG_AUTOSCAN_SETTING_WORD_GER` | 537 | 325 | `0x5F6724` | 999 |

**35. OSD2IMG_SYSTEM_SETTING_WORD：150×50，2 张保留 1 张，减少 7,500 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 225 | `OSD2IMG_SYSTEM_SETTING_WORD` | 537 | 150 | `0x22FB81` | 288 |
| 复用首张 | 752 | `OSD2IMG_SYSTEM_SETTING_WORD_GER` | 537 | 150 | `0x5EE830` | 995 |

**36. OSD2IMG_SYSTEM_DIMMER_ROUND_HL：48×48，4 张保留 1 张，减少 6,912 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 244 | `OSD2IMG_SYSTEM_DIMMER_ROUND_HL` | 500 | 164 | `0x2B292B` | 324 |
| 复用首张 | 286 | `OSD2IMG_SYSTEM_DAY_ROUND_HL` | 500 | 204 | `0x307A33` | 384 |
| 复用首张 | 287 | `OSD2IMG_SYSTEM_NIGHT_ROUND_HL` | 0 | 304 | `0x308333` | 385 |
| 复用首张 | 406 | `OSD2IMG_TIME_ZONE_ROUND_HL` | 310 | 360 | `0x421DE0` | 554 |

**37. OSD2IMG_PAIRCAM1HL_ICON_GER：100×60，2 张保留 1 张，减少 6,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 735 | `OSD2IMG_PAIRCAM1HL_ICON_GER` | 380 | 215 | `0x5D4E08` | 975 |
| 复用首张 | 743 | `OSD2IMG_DELCAM1HL_ICON_GER` | 380 | 315 | `0x5E0988` | 984 |

**38. OSD2IMG_PAIRCAM2NOR_ICON_GER：100×60，2 张保留 1 张，减少 6,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 736 | `OSD2IMG_PAIRCAM2NOR_ICON_GER` | 520 | 215 | `0x5D6578` | 976 |
| 复用首张 | 744 | `OSD2IMG_DELCAM2NOR_ICON_GER` | 520 | 315 | `0x5E20F8` | 985 |

**39. OSD2IMG_PAIRCAM1NOR_ICON_GER：100×60，2 张保留 1 张，减少 6,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 734 | `OSD2IMG_PAIRCAM1NOR_ICON_GER` | 380 | 215 | `0x5D3698` | 974 |
| 复用首张 | 742 | `OSD2IMG_DELCAM1NOR_ICON_GER` | 380 | 315 | `0x5DF218` | 983 |

**40. OSD2IMG_PAIRCAM1HL_ICON：100×60，2 张保留 1 张，减少 6,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 58 | `OSD2IMG_PAIRCAM1HL_ICON` | 380 | 215 | `0x1565A0` | 91 |
| 复用首张 | 66 | `OSD2IMG_DELCAM1HL_ICON` | 380 | 315 | `0x162120` | 100 |

**41. OSD2IMG_PAIRCAM1NOR_ICON：100×60，2 张保留 1 张，减少 6,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 57 | `OSD2IMG_PAIRCAM1NOR_ICON` | 380 | 215 | `0x154E30` | 90 |
| 复用首张 | 65 | `OSD2IMG_DELCAM1NOR_ICON` | 380 | 315 | `0x1609B0` | 99 |

**42. OSD2IMG_PAIRCAM4NOR_ICON_GER：100×60，2 张保留 1 张，减少 6,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 740 | `OSD2IMG_PAIRCAM4NOR_ICON_GER` | 800 | 215 | `0x5DC338` | 980 |
| 复用首张 | 748 | `OSD2IMG_DELCAM4NOR_ICON_GER` | 800 | 315 | `0x5E7EB8` | 989 |

**43. OSD2IMG_PAIRCAM3HL_ICON_GER：100×60，2 张保留 1 张，减少 6,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 739 | `OSD2IMG_PAIRCAM3HL_ICON_GER` | 660 | 215 | `0x5DABC8` | 979 |
| 复用首张 | 747 | `OSD2IMG_DELCAM3HL_ICON_GER` | 660 | 315 | `0x5E6748` | 988 |

**44. OSD2IMG_PAIRCAM4HL_ICON_GER：100×60，2 张保留 1 张，减少 6,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 741 | `OSD2IMG_PAIRCAM4HL_ICON_GER` | 800 | 215 | `0x5DDAA8` | 981 |
| 复用首张 | 749 | `OSD2IMG_DELCAM4HL_ICON_GER` | 800 | 315 | `0x5E9628` | 990 |

**45. OSD2IMG_PAIRCAM2HL_ICON_GER：100×60，2 张保留 1 张，减少 6,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 737 | `OSD2IMG_PAIRCAM2HL_ICON_GER` | 520 | 215 | `0x5D7CE8` | 977 |
| 复用首张 | 745 | `OSD2IMG_DELCAM2HL_ICON_GER` | 520 | 315 | `0x5E3868` | 986 |

**46. OSD2IMG_PAIRCAM3NOR_ICON_GER：100×60，2 张保留 1 张，减少 6,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 738 | `OSD2IMG_PAIRCAM3NOR_ICON_GER` | 660 | 215 | `0x5D9458` | 978 |
| 复用首张 | 746 | `OSD2IMG_DELCAM3NOR_ICON_GER` | 660 | 315 | `0x5E4FD8` | 987 |

**47. OSD2IMG_PAIRCAM3HL_ICON：100×60，2 张保留 1 张，减少 6,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 62 | `OSD2IMG_PAIRCAM3HL_ICON` | 660 | 215 | `0x15C360` | 95 |
| 复用首张 | 70 | `OSD2IMG_DELCAM3HL_ICON` | 660 | 315 | `0x167EE0` | 104 |

**48. OSD2IMG_PAIRCAM4NOR_ICON：100×60，2 张保留 1 张，减少 6,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 63 | `OSD2IMG_PAIRCAM4NOR_ICON` | 800 | 215 | `0x15DAD0` | 96 |
| 复用首张 | 71 | `OSD2IMG_DELCAM4NOR_ICON` | 800 | 315 | `0x169650` | 105 |

**49. OSD2IMG_PAIRCAM4HL_ICON：100×60，2 张保留 1 张，减少 6,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 64 | `OSD2IMG_PAIRCAM4HL_ICON` | 800 | 215 | `0x15F240` | 97 |
| 复用首张 | 72 | `OSD2IMG_DELCAM4HL_ICON` | 800 | 315 | `0x16ADC0` | 106 |

**50. OSD2IMG_PAIRCAM2NOR_ICON：100×60，2 张保留 1 张，减少 6,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 59 | `OSD2IMG_PAIRCAM2NOR_ICON` | 520 | 215 | `0x157D10` | 92 |
| 复用首张 | 67 | `OSD2IMG_DELCAM2NOR_ICON` | 520 | 315 | `0x163890` | 101 |

**51. OSD2IMG_PAIRCAM2HL_ICON：100×60，2 张保留 1 张，减少 6,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 60 | `OSD2IMG_PAIRCAM2HL_ICON` | 520 | 215 | `0x159480` | 93 |
| 复用首张 | 68 | `OSD2IMG_DELCAM2HL_ICON` | 520 | 315 | `0x165000` | 102 |

**52. OSD2IMG_PAIRCAM3NOR_ICON：100×60，2 张保留 1 张，减少 6,000 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 61 | `OSD2IMG_PAIRCAM3NOR_ICON` | 660 | 215 | `0x15ABF0` | 94 |
| 复用首张 | 69 | `OSD2IMG_DELCAM3NOR_ICON` | 660 | 315 | `0x166770` | 103 |

**53. OSD2IMG_CAMS_VOLUME_WORD：104×50，2 张保留 1 张，减少 5,200 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 325 | `OSD2IMG_CAMS_VOLUME_WORD` | 105 | 460 | `0x35E3F3` | 437 |
| 复用首张 | 866 | `OSD2IMG_CAMS_VOLUME_WORD_FR` | 96 | 460 | `0x7A3698` | 1167 |

**54. OSD2IMG_SOUND_ROUND_COVER：48×48，2 张保留 1 张，减少 2,304 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 35 | `OSD2IMG_SOUND_ROUND_COVER` | 300 | 150 | `0x1053FD` | 62 |
| 复用首张 | 337 | `OSD2IMG_CAMS_ROUND_COVER` | 0 | 0 | `0x374C87` | 452 |

**55. OSD2IMG_SYSTEM_DIMMER_ROUND_NOR：48×48，2 张保留 1 张，减少 2,304 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 245 | `OSD2IMG_SYSTEM_DIMMER_ROUND_NOR` | 500 | 164 | `0x2B322B` | 325 |
| 复用首张 | 407 | `OSD2IMG_TIME_ZONE_ROUND_NOR` | 310 | 360 | `0x4226E0` | 555 |

**56. OSD2IMG_SOUND_ROUND：48×48，2 张保留 1 张，减少 2,304 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 34 | `OSD2IMG_SOUND_ROUND` | 300 | 150 | `0x104AFD` | 61 |
| 复用首张 | 336 | `OSD2IMG_CAMS_ROUND_HL` | 0 | 0 | `0x374387` | 451 |

**57. OSD2IMG_SYSTEM_DIMMER_ROUND_COVER：48×48，2 张保留 1 张，减少 2,304 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 243 | `OSD2IMG_SYSTEM_DIMMER_ROUND_COVER` | 500 | 164 | `0x2B202B` | 323 |
| 复用首张 | 405 | `OSD2IMG_TIME_ZONE_ROUND_COVER` | 310 | 360 | `0x4214E0` | 553 |

**58. OSD2IMG_FILE_FILEBOX_UPPER：984×2，2 张保留 1 张，减少 1,968 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 172 | `OSD2IMG_FILE_FILEBOX_UPPER` | 20 | 80 | `0x1B818B` | 223 |
| 复用首张 | 173 | `OSD2IMG_FILE_FILEBOX_LOWER` | 20 | 128 | `0x1B893B` | 224 |

**59. OSD1IMG_DESKTOP_VOICE_MUTE：40×40，2 张保留 1 张，减少 1,600 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 35 | `OSD1IMG_DESKTOP_VOICE_MUTE` | 452 | 550 | `0xF5889` | 79 |
| 复用首张 | 17 | `OSD2IMG_DESKTOP_VOICE_MUTE` | 452 | 550 | `0xFDE15` | 41 |

**60. OSD1IMG_DESKTOP_VOICE_UNMUTE：40×40，2 张保留 1 张，减少 1,600 字节**

| 处理 | 层内编号 | 图片名 | x | y | 像素地址 | 资源表行号 |
| --- | ---: | --- | ---: | ---: | --- | ---: |
| 保留 | 36 | `OSD1IMG_DESKTOP_VOICE_UNMUTE` | 452 | 550 | `0xF5EC9` | 80 |
| 复用首张 | 18 | `OSD2IMG_DESKTOP_VOICE_UNMUTE` | 452 | 550 | `0xFE455` | 42 |
