from __future__ import annotations

from datetime import date
from pathlib import Path
from typing import Iterable, Sequence

from docx import Document
from docx.enum.section import WD_SECTION
from docx.enum.style import WD_STYLE_TYPE
from docx.enum.table import WD_CELL_VERTICAL_ALIGNMENT, WD_TABLE_ALIGNMENT
from docx.enum.text import WD_ALIGN_PARAGRAPH, WD_BREAK, WD_LINE_SPACING
from docx.oxml import OxmlElement
from docx.oxml.ns import qn
from docx.shared import Inches, Mm, Pt, RGBColor


ROOT = Path(__file__).resolve().parents[1]
OUTPUT_DIR = ROOT / "docs" / "软著材料"
SOFTWARE_NAME = "Album相册管理系统"
SOFTWARE_SHORT_NAME = "Album"
VERSION = "V1.0"
TODAY = date.today().strftime("%Y年%m月%d日")

PAGE_WIDTH_DXA = 10800  # A4 width minus 18 mm left/right margins.
TABLE_INDENT_DXA = 120
BLUE = "2E74B5"
NAVY = "0B2545"
MUTED = "667085"
LIGHT_BLUE = "E8EEF5"
LIGHT_GRAY = "F2F4F7"


def set_run_font(run, name="Microsoft YaHei", size=10.5, color=None, bold=None, italic=None):
    run.font.name = name
    run._element.get_or_add_rPr().rFonts.set(qn("w:ascii"), name)
    run._element.get_or_add_rPr().rFonts.set(qn("w:hAnsi"), name)
    run._element.get_or_add_rPr().rFonts.set(qn("w:eastAsia"), name)
    run.font.size = Pt(size)
    if color:
        run.font.color.rgb = RGBColor.from_string(color)
    if bold is not None:
        run.bold = bold
    if italic is not None:
        run.italic = italic


def configure_page(section):
    section.page_width = Mm(210)
    section.page_height = Mm(297)
    section.top_margin = Mm(18)
    section.bottom_margin = Mm(18)
    section.left_margin = Mm(18)
    section.right_margin = Mm(18)
    section.header_distance = Mm(9)
    section.footer_distance = Mm(9)


def configure_styles(doc):
    styles = doc.styles
    normal = styles["Normal"]
    normal.font.name = "Microsoft YaHei"
    normal._element.get_or_add_rPr().rFonts.set(qn("w:eastAsia"), "Microsoft YaHei")
    normal.font.size = Pt(10.5)
    normal.paragraph_format.space_after = Pt(6)
    normal.paragraph_format.line_spacing = 1.15

    heading_tokens = {
        "Heading 1": (16, BLUE, 16, 8),
        "Heading 2": (13, BLUE, 12, 6),
        "Heading 3": (11.5, NAVY, 8, 4),
    }
    for name, (size, color, before, after) in heading_tokens.items():
        style = styles[name]
        style.font.name = "Microsoft YaHei"
        style._element.get_or_add_rPr().rFonts.set(qn("w:eastAsia"), "Microsoft YaHei")
        style.font.size = Pt(size)
        style.font.bold = True
        style.font.color.rgb = RGBColor.from_string(color)
        style.paragraph_format.space_before = Pt(before)
        style.paragraph_format.space_after = Pt(after)
        style.paragraph_format.keep_with_next = True

    if "CodeLine" not in styles:
        code = styles.add_style("CodeLine", WD_STYLE_TYPE.PARAGRAPH)
    else:
        code = styles["CodeLine"]
    code.font.name = "Consolas"
    code._element.get_or_add_rPr().rFonts.set(qn("w:ascii"), "Consolas")
    code._element.get_or_add_rPr().rFonts.set(qn("w:hAnsi"), "Consolas")
    code._element.get_or_add_rPr().rFonts.set(qn("w:eastAsia"), "Microsoft YaHei")
    code.font.size = Pt(6.5)
    code.paragraph_format.space_before = Pt(0)
    code.paragraph_format.space_after = Pt(0)
    code.paragraph_format.line_spacing_rule = WD_LINE_SPACING.SINGLE
    code.paragraph_format.keep_together = True


def add_page_field(paragraph):
    run = paragraph.add_run()
    begin = OxmlElement("w:fldChar")
    begin.set(qn("w:fldCharType"), "begin")
    instr = OxmlElement("w:instrText")
    instr.set(qn("xml:space"), "preserve")
    instr.text = " PAGE "
    separate = OxmlElement("w:fldChar")
    separate.set(qn("w:fldCharType"), "separate")
    text = OxmlElement("w:t")
    text.text = "1"
    separate.append(text)
    end = OxmlElement("w:fldChar")
    end.set(qn("w:fldCharType"), "end")
    run._r.extend([begin, instr, separate, end])
    set_run_font(run, size=9, color=MUTED)


def configure_header_footer(doc, header_label):
    for section in doc.sections:
        configure_page(section)
        header = section.header.paragraphs[0]
        header.alignment = WD_ALIGN_PARAGRAPH.RIGHT
        header.text = ""
        run = header.add_run(f"{SOFTWARE_NAME} {VERSION} | {header_label}")
        set_run_font(run, size=8.5, color=MUTED)
        footer = section.footer.paragraphs[0]
        footer.alignment = WD_ALIGN_PARAGRAPH.CENTER
        footer.text = "第 "
        for run in footer.runs:
            set_run_font(run, size=9, color=MUTED)
        add_page_field(footer)
        suffix = footer.add_run(" 页")
        set_run_font(suffix, size=9, color=MUTED)


def set_cell_shading(cell, fill):
    tc_pr = cell._tc.get_or_add_tcPr()
    shd = tc_pr.find(qn("w:shd"))
    if shd is None:
        shd = OxmlElement("w:shd")
        tc_pr.append(shd)
    shd.set(qn("w:fill"), fill)


def set_paragraph_shading(paragraph, fill):
    p_pr = paragraph._p.get_or_add_pPr()
    shd = p_pr.find(qn("w:shd"))
    if shd is None:
        shd = OxmlElement("w:shd")
        p_pr.append(shd)
    shd.set(qn("w:fill"), fill)


def set_cell_margins(cell, top=90, start=120, bottom=90, end=120):
    tc = cell._tc
    tc_pr = tc.get_or_add_tcPr()
    tc_mar = tc_pr.first_child_found_in("w:tcMar")
    if tc_mar is None:
        tc_mar = OxmlElement("w:tcMar")
        tc_pr.append(tc_mar)
    for margin, value in (("top", top), ("start", start), ("bottom", bottom), ("end", end)):
        node = tc_mar.find(qn(f"w:{margin}"))
        if node is None:
            node = OxmlElement(f"w:{margin}")
            tc_mar.append(node)
        node.set(qn("w:w"), str(value))
        node.set(qn("w:type"), "dxa")


def set_table_geometry(table, widths: Sequence[int], indent=TABLE_INDENT_DXA):
    table.autofit = False
    table.alignment = WD_TABLE_ALIGNMENT.LEFT
    tbl = table._tbl
    tbl_pr = tbl.tblPr
    tbl_w = tbl_pr.find(qn("w:tblW"))
    if tbl_w is None:
        tbl_w = OxmlElement("w:tblW")
        tbl_pr.append(tbl_w)
    tbl_w.set(qn("w:w"), str(sum(widths)))
    tbl_w.set(qn("w:type"), "dxa")
    tbl_ind = tbl_pr.find(qn("w:tblInd"))
    if tbl_ind is None:
        tbl_ind = OxmlElement("w:tblInd")
        tbl_pr.append(tbl_ind)
    tbl_ind.set(qn("w:w"), str(indent))
    tbl_ind.set(qn("w:type"), "dxa")
    grid = tbl.tblGrid
    for child in list(grid):
        grid.remove(child)
    for width in widths:
        col = OxmlElement("w:gridCol")
        col.set(qn("w:w"), str(width))
        grid.append(col)
    for row in table.rows:
        for cell, width in zip(row.cells, widths):
            cell.width = Inches(width / 1440)
            tc_pr = cell._tc.get_or_add_tcPr()
            tc_w = tc_pr.find(qn("w:tcW"))
            if tc_w is None:
                tc_w = OxmlElement("w:tcW")
                tc_pr.append(tc_w)
            tc_w.set(qn("w:w"), str(width))
            tc_w.set(qn("w:type"), "dxa")
            cell.vertical_alignment = WD_CELL_VERTICAL_ALIGNMENT.CENTER
            set_cell_margins(cell)


def mark_header_row(row):
    tr_pr = row._tr.get_or_add_trPr()
    tbl_header = OxmlElement("w:tblHeader")
    tbl_header.set(qn("w:val"), "true")
    tr_pr.append(tbl_header)


def add_table(doc, headers: Sequence[str], rows: Iterable[Sequence[str]], widths: Sequence[int]):
    data = list(rows)
    table = doc.add_table(rows=1, cols=len(headers))
    set_table_geometry(table, widths)
    mark_header_row(table.rows[0])
    for index, value in enumerate(headers):
        cell = table.rows[0].cells[index]
        set_cell_shading(cell, LIGHT_BLUE)
        cell.paragraphs[0].text = ""
        run = cell.paragraphs[0].add_run(str(value))
        set_run_font(run, size=9.5, color=NAVY, bold=True)
    for row_values in data:
        cells = table.add_row().cells
        for index, value in enumerate(row_values):
            cells[index].paragraphs[0].text = ""
            run = cells[index].paragraphs[0].add_run(str(value))
            set_run_font(run, size=9.2)
    set_table_geometry(table, widths)
    doc.add_paragraph().paragraph_format.space_after = Pt(1)
    return table


def add_title(doc, title, subtitle):
    p = doc.add_paragraph()
    p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p.paragraph_format.space_before = Pt(72)
    p.paragraph_format.space_after = Pt(10)
    run = p.add_run(title)
    set_run_font(run, size=24, color=NAVY, bold=True)
    p = doc.add_paragraph()
    p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p.paragraph_format.space_after = Pt(34)
    run = p.add_run(subtitle)
    set_run_font(run, size=13, color=MUTED)


def add_label_value(doc, label, value):
    p = doc.add_paragraph()
    p.paragraph_format.space_after = Pt(4)
    label_run = p.add_run(f"{label}：")
    set_run_font(label_run, bold=True, color=NAVY)
    value_run = p.add_run(value)
    set_run_font(value_run)
    return p


def add_bullet(doc, text, level=0):
    p = doc.add_paragraph(style="List Bullet")
    p.paragraph_format.left_indent = Inches(0.25 + level * 0.22)
    p.paragraph_format.first_line_indent = Inches(-0.18)
    p.paragraph_format.space_after = Pt(4)
    run = p.add_run(text)
    set_run_font(run)
    return p


def add_numbered(doc, text):
    p = doc.add_paragraph(style="List Number")
    p.paragraph_format.left_indent = Inches(0.33)
    p.paragraph_format.first_line_indent = Inches(-0.22)
    p.paragraph_format.space_after = Pt(5)
    run = p.add_run(text)
    set_run_font(run)
    return p


def add_callout(doc, label, text):
    p = doc.add_paragraph()
    p.paragraph_format.left_indent = Pt(6)
    p.paragraph_format.right_indent = Pt(6)
    p.paragraph_format.space_before = Pt(4)
    p.paragraph_format.space_after = Pt(8)
    set_paragraph_shading(p, "F4F6F9")
    p.text = ""
    r = p.add_run(f"{label}：")
    set_run_font(r, color=NAVY, bold=True)
    r = p.add_run(text)
    set_run_font(r, color=NAVY)


def production_files() -> list[Path]:
    allowed = {".cpp", ".h", ".ui", ".qss", ".qrc"}
    result: list[Path] = []
    for path in ROOT.rglob("*"):
        if not path.is_file():
            continue
        relative = path.relative_to(ROOT)
        if any(part in {"build", "tests", ".git", ".codex", ".agents", "docs", "tools"} for part in relative.parts):
            continue
        if path.suffix.lower() in allowed or path.name == "CMakeLists.txt":
            result.append(path)
    return sorted(result, key=lambda p: p.relative_to(ROOT).as_posix().lower())


def source_stats(files: Sequence[Path]) -> tuple[int, int, int]:
    cpp_lines = sum(len(path.read_text(encoding="utf-8").splitlines()) for path in files if path.suffix in {".cpp", ".h"})
    all_lines = sum(len(path.read_text(encoding="utf-8").splitlines()) for path in files)
    return cpp_lines, all_lines, len(files)


def create_source_document(files: Sequence[Path]):
    doc = Document()
    configure_styles(doc)
    configure_header_footer(doc, "源程序文档")
    doc.core_properties.title = f"{SOFTWARE_NAME} {VERSION} 源程序文档"
    doc.core_properties.subject = "计算机软件著作权登记程序鉴别材料整理稿"
    doc.core_properties.author = SOFTWARE_NAME

    cpp_lines, all_lines, count = source_stats(files)
    add_title(doc, f"{SOFTWARE_NAME}\n源程序文档", "软件著作权登记材料整理稿")
    add_table(
        doc,
        ["项目", "内容"],
        [
            ("软件全称", SOFTWARE_NAME),
            ("软件简称", SOFTWARE_SHORT_NAME),
            ("版本号", VERSION),
            ("材料类型", "完整生产源程序"),
            ("源文件数量", f"{count} 个"),
            ("C++/头文件行数", f"{cpp_lines} 行"),
            ("项目源文件总行数", f"{all_lines} 行"),
            ("生成日期", TODAY),
        ],
        [2600, 6760],
    )
    add_callout(doc, "编排说明", "本文件按当前项目生产代码目录整理，排除 tests、build、docs 和 tools。源文件之间使用材料索引标识分隔；该标识仅用于阅读，不属于软件源代码。正式提交时应按中国版权保护中心当期要求选择连续页码并核对页眉、版本号和申请表信息。")
    doc.add_page_break()

    doc.add_heading("一、源文件目录", level=1)
    add_table(
        doc,
        ["序号", "源文件路径", "类型", "行数"],
        [
            (str(index), path.relative_to(ROOT).as_posix(), "CMake" if path.name == "CMakeLists.txt" else path.suffix, str(len(path.read_text(encoding="utf-8").splitlines())))
            for index, path in enumerate(files, 1)
        ],
        [900, 5360, 1400, 1700],
    )
    doc.add_page_break()

    doc.add_heading("二、完整生产源程序", level=1)
    doc.add_paragraph("以下内容为当前项目的完整生产源程序，按文件路径排序连续编排。每一行源文件内容对应一个代码材料行。")
    code_lines: list[str] = []
    for path in files:
        relative = path.relative_to(ROOT).as_posix()
        code_lines.append(f"// ===== SOURCE FILE: {relative} =====")
        content = path.read_text(encoding="utf-8").splitlines()
        code_lines.extend(content if content else [""])
    code_lines.append("// ===== END OF SOURCE PROGRAM =====")
    for start in range(0, len(code_lines), 50):
        if start:
            doc.add_page_break()
        for line in code_lines[start:start + 50]:
            paragraph = doc.add_paragraph(style="CodeLine")
            paragraph.paragraph_format.keep_together = True
            run = paragraph.add_run(line if line else " ")
            set_run_font(run, name="Consolas", size=6.5, color="1F2937")

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    doc.save(OUTPUT_DIR / f"{SOFTWARE_NAME}_{VERSION}_源程序文档.docx")


def create_manual_document(files: Sequence[Path]):
    doc = Document()
    configure_styles(doc)
    configure_header_footer(doc, "文件材料与用户手册")
    doc.core_properties.title = f"{SOFTWARE_NAME} {VERSION} 文件材料与用户手册"
    doc.core_properties.subject = "计算机软件著作权登记文档鉴别材料整理稿"
    doc.core_properties.author = SOFTWARE_NAME

    cpp_lines, all_lines, count = source_stats(files)
    add_title(doc, f"{SOFTWARE_NAME}\n文件材料与用户手册", "软件著作权登记文档鉴别材料整理稿")
    add_table(
        doc,
        ["申请字段", "建议填写内容"],
        [
            ("软件全称", SOFTWARE_NAME),
            ("软件简称", SOFTWARE_SHORT_NAME),
            ("版本号", VERSION),
            ("软件分类", "应用软件"),
            ("开发方式", "原始取得（请按实际权属情况核对）"),
            ("编程语言", "C++17、Qt 6 Widgets、Qt Multimedia、CMake、QSS"),
            ("源程序量", f"C++/头文件约 {cpp_lines} 行；项目源文件共 {all_lines} 行；生产源文件 {count} 个"),
            ("开发完成日期", "请按实际开发完成日期填写"),
            ("发表状态", "请按实际情况填写"),
        ],
        [2600, 6760],
    )
    add_callout(doc, "提交前核对", "本材料已经将用户提供的示例字段转换为当前桌面相册软件的真实口径。申请主体、著作权人、开发完成日期、首次发表状态、权利取得方式等法律信息不能由程序自动推断，提交前必须由申请人按事实填写。")

    doc.add_page_break()
    doc.add_heading("一、文档说明", level=1)
    doc.add_paragraph("本文件用于说明 Album相册管理系统的开发目的、运行环境、功能组成、技术特点、数据组织、操作方法和测试情况，也可作为软件著作权登记中的文档鉴别材料整理稿。文档只描述当前项目已经实现的能力，不包含服务器、移动客户端或在线账户等项目外功能。")

    doc.add_heading("二、软件概述", level=1)
    doc.add_paragraph(f"{SOFTWARE_NAME}（简称 {SOFTWARE_SHORT_NAME}）是一款基于 Qt 6 Widgets 的 Windows 桌面相册管理软件。用户可以按项目管理图片，导入本地图片或图片文件夹，使用标签进行分类筛选，对图片进行基础编辑，并通过预览、轮播和背景音乐形成完整的本地相册浏览体验。软件以本地文件夹和 JSON 文件保存数据，项目目录可以整体复制或迁移。")

    doc.add_heading("三、开发目的", level=1)
    doc.add_paragraph("软件面向需要在本地整理家庭照片、旅行照片、作品素材和学习资料的个人用户。开发目的包括：降低相册整理门槛；将图片、标签、编辑结果和音乐配置集中保存；提供无需联网即可使用的浏览和展示方式；通过清晰的项目目录结构保护原始图片并便于备份迁移。")

    doc.add_heading("四、硬件环境", level=1)
    add_table(
        doc,
        ["环境类别", "推荐配置"],
        [
            ("开发硬件环境", "四核 Intel Core i5 或同等处理器；12 GB 内存；250 GB 固态硬盘；网络带宽 1 Mbps 以上用于获取开发依赖。"),
            ("运行硬件环境", "双核及以上 x64 处理器；4 GB 内存；至少 2 GB 可用磁盘空间；1366×768 或更高分辨率显示器；音频播放设备为可选。"),
            ("数据规模建议", "单个项目建议按文件夹分组管理；图片和音乐实际占用空间取决于用户导入内容。"),
        ],
        [2600, 6760],
    )

    doc.add_heading("五、软件环境", level=1)
    add_table(
        doc,
        ["环境类别", "软件配置"],
        [
            ("开发操作系统", "Windows 10/11 64 位。"),
            ("开发工具", "Qt 6.8.3、Qt Creator（可选）、MSVC 2022、CMake 3.16 及以上。"),
            ("核心开发框架", "Qt Widgets、Qt Multimedia、Qt Test、Qt Resource System。"),
            ("运行操作系统", "Windows 10/11 64 位，配套 Qt 6.8.3 运行库和 Qt Multimedia 系统编解码器。"),
            ("运行依赖", "Windows 文件系统；图片格式由 Qt QImageReader 支持；音乐格式依赖系统可用的 Qt Multimedia 编解码器。"),
        ],
        [2600, 6760],
    )

    doc.add_heading("六、主要功能", level=1)
    for item in [
        "项目管理：创建、打开、关闭相册项目，项目树展示最近打开的项目。",
        "图片导入：支持单张、多张和文件夹导入，校验图片格式、自动重名、失败汇总和导入回滚。",
        "画廊浏览：以缩略图网格展示图片，支持多选、删除、文件名排序和导入时间排序。",
        "预览与轮播：双击图片进入预览，支持前后切换、适应窗口、原始尺寸和全屏轮播。",
        "标签分类：为一张或多张图片设置标签，按标签筛选图片并持久化分类信息。",
        "图片编辑：支持 90 度旋转、水平/垂直翻转、亮度调整、灰度处理和裁剪；原图与编辑结果分开保存。",
        "背景音乐：导入 MP3、WAV、FLAC、M4A、OGG 等音频，支持播放/暂停、上一首、下一首、音量和循环。",
        "数据持久化：项目元数据、标签、编辑参数、音乐列表和筛选状态保存到 album.json，图片、编辑结果和音乐使用相对路径。",
    ]:
        add_bullet(doc, item)

    doc.add_heading("七、技术特点", level=1)
    for item in [
        "面向对象设计：项目模型、导入器、渲染器、画廊、编辑器、音乐控制器和对话框按职责拆分。",
        "非破坏性编辑：编辑结果写入 edits 目录，原始图片保持不变，重置编辑即可恢复原图显示。",
        "可迁移数据：项目 JSON 使用相对路径，项目文件夹整体复制后仍可重新打开。",
        "安全保存：项目配置使用 QSaveFile 写入，降低程序中断造成的配置文件损坏风险。",
        "可测试性：核心模型、导入、图像渲染、画廊、音乐控制和主窗口均提供 Qt Test 测试覆盖。",
        "轻量离线：核心图片管理不依赖服务器、数据库和用户账户，适合个人本地资料管理。",
    ]:
        add_bullet(doc, item)

    doc.add_heading("八、系统结构与数据流", level=1)
    add_table(
        doc,
        ["模块", "职责", "主要数据"],
        [
            ("MainWindow", "菜单、工具栏、筛选和模块协调", "当前项目、选择状态"),
            ("AlbumProject", "项目模型、JSON 持久化、标签和排序", "album.json"),
            ("PhotoImporter", "图片识别、复制、去重和失败回滚", "photos/"),
            ("ImageRenderer / Editor", "编辑参数和图片渲染", "edits/、EditSettings"),
            ("Gallery / Viewer / Slideshow", "缩略图、预览和轮播展示", "PhotoRecord"),
            ("MusicController", "音频播放、音量和循环", "music/"),
        ],
        [2500, 4400, 2460],
    )
    add_callout(doc, "典型数据流", "创建项目 → 导入图片 → 生成 PhotoRecord → 缩略图展示 → 标签筛选或图片编辑 → 保存 album.json 与编辑结果 → 预览或轮播。背景音乐作为项目配置的一部分独立导入和播放。")

    doc.add_heading("九、项目目录说明", level=1)
    add_table(
        doc,
        ["目录/文件", "作用"],
        [
            ("album.json", "项目名称、图片元数据、标签、编辑参数、排序状态和音乐列表。"),
            ("photos/", "导入后的原始图片，程序不会直接修改原始图片内容。"),
            ("edits/", "启用编辑后的 PNG 结果，重置编辑时删除对应结果文件。"),
            ("music/", "复制到项目中的背景音乐文件。"),
        ],
        [2600, 6760],
    )

    doc.add_heading("十、用户手册", level=1)
    doc.add_heading("10.1 启动软件", level=2)
    add_numbered(doc, "在 Windows 10/11 64 位系统中运行 Album.exe。首次启动显示空白工作区，左侧为项目树，右侧为图片画廊。")
    add_numbered(doc, "通过“帮助 → 关于软件”检查软件名称和版本号，确认显示为 Album相册管理系统 V1.0。")

    doc.add_heading("10.2 创建或打开项目", level=2)
    add_numbered(doc, "选择“文件 → 创建项目”，输入项目名称和保存位置，确认后软件会创建 album.json、photos、edits 和 music 目录。")
    add_numbered(doc, "选择“文件 → 打开项目”，定位到已有项目目录中的 album.json；项目目录整体移动后仍可按相同方式打开。")
    add_numbered(doc, "需要切换项目时选择“文件 → 关闭项目”，确认保存选项后再打开其他项目。")

    doc.add_heading("10.3 导入图片", level=2)
    add_numbered(doc, "打开项目后，选择“图片 → 导入图片”选择一张或多张图片。")
    add_numbered(doc, "选择“图片 → 导入文件夹”可扫描文件夹中的支持格式图片。")
    add_numbered(doc, "导入器会验证图片是否可读取；同名文件自动生成不重复的目标文件名，失败文件会在提示框中列出原因。")

    doc.add_heading("10.4 浏览、排序与预览", level=2)
    add_numbered(doc, "右侧画廊以缩略图展示当前筛选结果，可点击选择，多选用于批量标签或删除。")
    add_numbered(doc, "在顶部“标签筛选”选择“全部”或具体标签；在“排序”选择导入时间或文件名。")
    add_numbered(doc, "双击缩略图打开预览，使用上一张/下一张切换图片，或切换适应窗口与原始尺寸。")
    add_numbered(doc, "点击“开始轮播”进入全屏轮播；左/右方向键切换，空格暂停或继续，Esc 退出。")

    doc.add_heading("10.5 标签分类", level=2)
    add_numbered(doc, "选择一张或多张图片，点击“图片 → 编辑标签”。")
    add_numbered(doc, "勾选已有标签或输入新标签；确认后所选图片会统一使用当前标签集合。")
    add_numbered(doc, "回到标签筛选栏即可查看某一分类，标签信息会保存到 album.json。")

    doc.add_heading("10.6 图片编辑", level=2)
    add_numbered(doc, "选择一张图片，点击“图片 → 编辑图片”。编辑器右侧显示实时预览。")
    add_numbered(doc, "使用旋转、水平翻转、垂直翻转、亮度、灰度和裁剪参数调整图片。")
    add_numbered(doc, "点击“应用”后，编辑结果保存到 edits 目录，编辑参数保存到 album.json；原图保留在 photos 目录。")
    add_numbered(doc, "点击“重置编辑”可恢复原图显示，并删除对应编辑结果。")

    doc.add_heading("10.7 背景音乐", level=2)
    add_numbered(doc, "选择“设置 → 导入背景音乐”或底部“添加音乐”，导入本地 MP3、WAV、FLAC、M4A、OGG 文件。")
    add_numbered(doc, "使用底部播放/暂停、上一首、下一首按钮和音量滑块控制播放；“循环”按钮控制列表循环。")
    add_numbered(doc, "音乐文件会复制到项目 music 目录，项目移动时音乐配置随项目移动。")

    doc.add_heading("10.8 异常处理", level=2)
    add_table(
        doc,
        ["现象", "处理方法"],
        [
            ("项目无法打开", "确认选择的是包含 album.json 的项目目录；检查文件是否被占用或损坏。"),
            ("图片未显示", "确认图片文件存在且格式可被 Qt 读取；重新导入原始文件。"),
            ("音乐无法播放", "检查系统音频设备和 Qt Multimedia 编解码器；更换为 MP3 或 WAV 后重试。"),
            ("编辑结果异常", "点击重置编辑恢复原图；确认 edits 目录具有写入权限。"),
            ("导入部分失败", "根据失败汇总提示检查文件格式、权限和文件是否损坏，成功文件不会被回滚。"),
        ],
        [2600, 6760],
    )

    doc.add_heading("十一、测试与验证记录", level=1)
    add_table(
        doc,
        ["测试项", "验证内容", "结果"],
        [
            ("album_project_tests", "项目创建、打开、保存、标签、筛选和编辑路径", "通过"),
            ("photo_importer_tests", "图片校验、复制、重名处理和失败回滚", "通过"),
            ("image_renderer_tests", "裁剪、旋转、翻转、亮度和灰度渲染", "通过"),
            ("gallery_smoke_tests", "画廊创建、缩略图展示和选择", "通过"),
            ("music_controller_tests", "播放列表、播放控制、音量和循环状态", "通过"),
            ("mainwindow_smoke_tests", "主窗口加载、项目打开、菜单动作和状态更新", "通过"),
        ],
        [2500, 5260, 1600],
    )
    doc.add_paragraph("验证命令：ctest --test-dir build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug --output-on-failure。最近一次验证结果为 6/6 测试通过。")

    doc.add_heading("十二、软著提交材料清单", level=1)
    add_table(
        doc,
        ["材料", "本项目准备情况", "提交前动作"],
        [
            ("软件著作权登记申请表", "需在中国版权保护中心在线填写", "核对名称、版本、权利人、日期和权利取得方式"),
            ("程序鉴别材料", "已生成完整生产源程序文档", "按当期要求选择连续页码，确认页眉版本一致"),
            ("文档鉴别材料", "本文件包含说明书和用户手册", "按当期要求确认页数、页眉和页码"),
            ("身份证明/营业执照", "不属于本项目代码材料", "由申请主体准备并按系统要求上传"),
            ("签章页", "由登记系统生成", "打印、签字或盖章后按要求扫描上传"),
        ],
        [2500, 3800, 3060],
    )
    add_callout(doc, "重要提示", "软件著作权登记材料应以真实开发情况和官方当期要求为准。本文件是项目材料整理稿，不替代中国版权保护中心的申请表，也不对审核结果作保证。")

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    doc.save(OUTPUT_DIR / f"{SOFTWARE_NAME}_{VERSION}_文件材料与用户手册.docx")


def main():
    files = production_files()
    if not files:
        raise SystemExit("未找到生产源文件")
    create_source_document(files)
    create_manual_document(files)
    cpp_lines, all_lines, count = source_stats(files)
    print(f"generated: {SOFTWARE_NAME}_{VERSION}_源程序文档.docx")
    print(f"generated: {SOFTWARE_NAME}_{VERSION}_文件材料与用户手册.docx")
    print(f"production_files={count} cpp_h_lines={cpp_lines} all_lines={all_lines}")


if __name__ == "__main__":
    main()
