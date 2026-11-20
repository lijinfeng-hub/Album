# Album 软著材料整理实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 统一 Album 相册管理系统的软件元数据，并生成可供软著整理使用的源程序文档和文件材料与用户手册文档。

**Architecture:** 在 Qt 应用中增加单一元数据来源，由主窗口、关于窗口、CMake 和 README 共同引用；文档生成脚本读取生产源文件并统计行数，使用 python-docx 生成两个独立 DOCX，再用 LibreOffice 渲染为 PNG 进行逐页检查。

**Tech Stack:** C++17, Qt 6 Widgets/Multimedia, CMake, Python 3, python-docx, LibreOffice headless renderer.

## Global Constraints

- 软件全称固定为 `Album相册管理系统`。
- 软件简称固定为 `Album`。
- 版本号固定为 `V1.0`，CMake 工程版本使用 `1.0.0`。
- 文档只描述当前项目真实存在的功能和环境。
- 交付两个 DOCX；用户手册作为文件材料文档的一部分。

---

### Task 1: 统一软件元数据

**Files:**
- Create: `appmetadata.h`
- Modify: `CMakeLists.txt`, `main.cpp`, `mainwindow.cpp`, `mainwindow.h`

- [ ] **Step 1:** 增加名称、简称、版本和版权信息常量。
- [ ] **Step 2:** 将主窗口标题和关于入口绑定到统一常量。
- [ ] **Step 3:** 将 CMake 工程版本改为 `1.0.0`。
- [ ] **Step 4:** 编译并运行主窗口 smoke test。

### Task 2: 更新项目说明

**Files:**
- Modify: `README.md`

- [ ] **Step 1:** 补充软件全称、简称、版本、分类和源程序统计口径。
- [ ] **Step 2:** 补充软著材料与真实信息核对提示。
- [ ] **Step 3:** 扫描 README 和源码中的占位符及旧软件名称。

### Task 3: 生成源程序文档

**Files:**
- Create: `tools/generate_softcopyright_docs.py`
- Create: `docs/软著材料/Album相册管理系统_V1.0_源程序文档.docx`

- [ ] **Step 1:** 按生产代码清单读取源文件，排除 build 和 tests。
- [ ] **Step 2:** 生成文件索引、完整代码正文、页眉和页码。
- [ ] **Step 3:** 写入代码统计和提交提示。
- [ ] **Step 4:** 使用 `render_docx.py` 渲染并检查每页 PNG。

### Task 4: 生成文件材料与用户手册

**Files:**
- Modify: `tools/generate_softcopyright_docs.py`
- Create: `docs/软著材料/Album相册管理系统_V1.0_文件材料与用户手册.docx`

- [ ] **Step 1:** 写入申请字段、环境、目的、功能、技术特点和结构说明。
- [ ] **Step 2:** 写入安装、创建项目、导入、标签、编辑、音乐、轮播和故障处理步骤。
- [ ] **Step 3:** 写入测试记录和真实信息核对提示。
- [ ] **Step 4:** 渲染并检查全部页面，修复布局问题后再输出。

### Task 5: 最终核验

- [ ] **Step 1:** 执行 Debug 构建和全量 CTest。
- [ ] **Step 2:** 对两个 DOCX 做文本、页数、文件大小和渲染结果检查。
- [ ] **Step 3:** 确认最终交付目录只保留两个 DOCX 及必要的生成脚本。
