# 🏫 Campus Lost & Found Manager

<p align="center">
  <img src="https://img.shields.io/badge/Language-C-A8B9CC?style=for-the-badge&logo=c&logoColor=white" alt="C">
  <img src="https://img.shields.io/badge/GUI-GTK%204-4A86CF?style=for-the-badge&logo=gtk&logoColor=white" alt="GTK 4">
  <img src="https://img.shields.io/badge/Graphics-Cairo-2457F5?style=for-the-badge" alt="Cairo">
  <img src="https://img.shields.io/badge/Styling-GTK%20CSS-6478D0?style=for-the-badge" alt="GTK CSS">
</p>

<p align="center">
  <strong>Lost something? Found something? Help it find its way home.</strong>
  <br>
  A modern desktop application that brings campus lost-and-found reporting, item tracking, and recovery analytics together in one clean, interactive dashboard.
</p>

<p align="center">
  <a href="#-overview">Overview</a> •
  <a href="#-features">Features</a> •
  <a href="#-technology-stack">Tech Stack</a> •
  <a href="#-installation--setup">Installation</a> •
  <a href="#-how-it-works">How It Works</a>
</p>

---

## 📖 Overview

**Campus Lost & Found Manager — Dashboard Edition** is a C-based desktop application developed using **GTK 4** to simplify the process of reporting, tracking, and managing lost and found belongings within a college campus.

From calculators left in engineering blocks to books forgotten in the library, the application provides a centralized interface where help desk operators can record items, search existing reports, filter records by status or category, and track successful recoveries.

Built with a structured C implementation, a custom GTK CSS theme, and Cairo-powered graphics, the application combines practical record management with a polished dashboard experience.

Instead of relying on a complicated database setup, the project uses lightweight local text-file storage to preserve records between application sessions.

### 🎯 Project Objectives

- 🏫 **Centralize campus reports** — Maintain lost and found records in one accessible application.
- 🔎 **Simplify item discovery** — Find reports using keywords, categories, locations, and status filters.
- 📊 **Monitor recovery progress** — Understand report distribution and claimed-item statistics at a glance.
- 💾 **Preserve records locally** — Save changes to a text file without requiring a database server.
- 🎨 **Deliver a modern desktop experience** — Combine GTK widgets, custom styling, and graphical analytics in a native application.

---

## ✨ Features

<table>
  <tr>
    <td width="50%">
      <h3>📊 Interactive Dashboard</h3>
      View four live summary cards displaying total reports, lost items, found items, and claimed items. Counts and percentages update when records change.
    </td>
    <td width="50%">
      <h3>🔍 Instant Search & Filtering</h3>
      Search item names, categories, and locations while filtering records by Lost, Found, Claimed, or All Reports.
    </td>
  </tr>
  <tr>
    <td width="50%">
      <h3>📈 Category Analytics</h3>
      Explore a dynamic bar chart covering Electronics, Accessories, Books, Documents, and Other, with the highest-count category highlighted.
    </td>
    <td width="50%">
      <h3>🎯 Recovery Progress Gauge</h3>
      See the percentage of reports marked as claimed through a custom half-circle gauge rendered using Cairo graphics.
    </td>
  </tr>
  <tr>
    <td width="50%">
      <h3>➕ Report Lost or Found Items</h3>
      Create reports through a dedicated pop-up form with category and report-type selections, required-field checks, and input-length validation.
    </td>
    <td width="50%">
      <h3>✅ Claim & Delete Actions</h3>
      Mark unresolved reports as claimed or remove records that are no longer needed. Dashboard data refreshes after each change.
    </td>
  </tr>
  <tr>
    <td width="50%">
      <h3>💾 Automatic Data Persistence</h3>
      Save report records to <code>data/items.txt</code> after additions, claim updates, and deletions, allowing data to be loaded again at startup.
    </td>
    <td width="50%">
      <h3>🎨 Custom Dashboard Interface</h3>
      Use a royal-blue visual theme, styled navigation, status badges, category icons, rounded cards, hover states, and scrolling report lists.
    </td>
  </tr>
</table>

---

## 📊 Dashboard & Analytics

The application combines report management and visual analytics in a single window, making important information accessible without navigating through multiple screens.

### 📌 Dashboard Statistics

| Metric | Description |
|:--|:--|
| 📋 **Total Reports** | Number of item reports currently stored |
| 🔴 **Still Lost** | Reports whose status is Lost |
| 🔵 **Found, Waiting** | Reports whose status is Found |
| 🟢 **Claimed** | Reports marked as successfully claimed |

The dashboard also displays the distribution of reports by status in the sidebar, helping users quickly switch between different groups of records.

### 📈 Visual Analytics

**1. Reports by Category**

A bar chart compares the number of reports belonging to each supported category. Bar heights are calculated dynamically from the current records, and the category with the highest count receives a visual highlight.

**2. Recovery Progress**

A custom half-circle gauge displays the percentage of all reports that have been marked as claimed.

The calculation is:

`Recovery Percentage = (Claimed Reports / Total Reports) × 100`

If no reports exist, the application displays a zero percentage rather than attempting to divide by zero.

**3. Status Legend**

A color-coded legend summarizes claimed, found, and lost reports, making the recovery gauge easier to interpret.

> 💡 **What makes the visualizations special?** The recovery gauge is not an embedded chart image. It is drawn programmatically using Cairo, with its arc segments updated from the application's current report statistics.

---

## 🗂️ Supported Categories & Report Statuses

### Item Categories

| Category | Example |
|:--|:--|
| 💻 Electronics | Calculator, phone, headphones |
| 🎒 Accessories | Backpack, watch, umbrella |
| 📚 Books | Textbooks, notebooks |
| 📄 Documents | Student ID card, fee receipt |
| 📦 Other | Items that do not fit another category |

### Report Statuses

- 🔴 **Lost** — An item has been reported missing.
- 🔵 **Found** — An item has been found and recorded.
- 🟢 **Claimed** — The report has been marked as resolved.

The Add Report form supports Lost and Found as initial report types. Existing reports can subsequently be marked as Claimed using the corresponding action button.

---

## 🛠️ Technology Stack

| Technology | Purpose |
|:--|:--|
| **C (C99 or later)** | Application logic, data structures, calculations, and file operations |
| **GTK 4** | Desktop GUI, widgets, dialogs, navigation, and application window |
| **Cairo** | Programmatic drawing of the recovery progress gauge |
| **GTK CSS** | Custom colors, typography, card styling, buttons, and visual states |
| **Standard C I/O** | Reading and writing persistent report records |
| **GLib / GObject** | GTK application infrastructure, callbacks, string utilities, and widget signals |
| **Plain-text storage** | Local tab-separated records in `data/items.txt` |
| **GCC** | Compiling and linking the application |

### 🧩 Core C Concepts Used

This project also demonstrates several fundamental C programming concepts in a practical application:

- **Structures (`struct`)** — Represent each item through its ID, name, category, location, and status.
- **Arrays** — Store and manage up to 500 item records in memory.
- **Functions** — Separate the interface, record management, calculations, and file operations into reusable routines.
- **Pointers** — Work with GTK widgets, item records, and callback arguments.
- **String handling** — Validate input, compare statuses, search text, and format displayed information.
- **File handling** — Read saved records using `fopen()` and `fgets()`, and write updates using `fprintf()`.
- **Conditional compilation** — Use a compile-time setting to control whether sample records are loaded.
- **Event-driven programming** — Respond to button clicks, search changes, and other GTK signals.
- **Graphics programming** — Draw the recovery gauge using Cairo's drawing functions.

---

## 🚀 Installation & Setup

Follow the instructions for your operating system to install the required development tools, compile the source code, and launch the application.

### 1. Prerequisites

You will need:

- GCC or another compatible C compiler.
- GTK 4 development libraries.
- `pkg-config` for obtaining GTK compilation and linking flags.
- A terminal or command-line environment.

### 🐧 Ubuntu / Debian

Install the required packages:

```bash
sudo apt update
sudo apt install build-essential pkg-config libgtk-4-dev
```

### 🎩 Fedora / RHEL

```bash
sudo dnf install gcc pkgconf-pkg-config gtk4-devel
```

### 🍎 macOS

If you use Homebrew:

```bash
brew install gcc pkg-config gtk4
```

### 🪟 Windows — MSYS2 UCRT64

If you are using MSYS2, open the **UCRT64 terminal** and install the required packages:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-gtk4 mingw-w64-ucrt-x86_64-pkgconf
```

Run the following commands in the UCRT64 terminal, not in ordinary PowerShell.

---

### 2. Clone the Repository

```bash
git clone https://github.com/your-username/campus-lost-found.git
cd campus-lost-found
```

Replace the placeholder repository URL with your actual GitHub repository address.

### 3. Verify the Project Files

Make sure the main C source file and stylesheet are present in the project directory:

```text
main.c
style.css
```

The application loads its CSS stylesheet from the current working directory. Run the executable from the project directory so that the styling file can be located correctly.

### 4. Compile the Application

Use GCC with `pkg-config` to supply the GTK 4 compiler and linker flags:

```bash
gcc main.c -o campus_lost_found $(pkg-config --cflags --libs gtk4)
```

If compilation succeeds, the `campus_lost_found` executable will be created in the current directory.

### 5. Run the Application

**Linux / macOS:**

```bash
./campus_lost_found
```

**Windows — MSYS2 UCRT64:**

```bash
./campus_lost_found.exe
```

On Windows, ensure the required GTK runtime libraries are available through the MSYS2 UCRT64 environment.

---

## 📂 Project Structure

```text
Campus-Lost-Found-Manager/
│
├── main.c                 # Main C application
│                          # GTK interface, callbacks, logic and file I/O
│
├── style.css              # GTK CSS theme and widget styling
│
├── data/
│   └── items.txt          # Persistent tab-separated item records
│
├── campus_lost_found      # Compiled executable (platform-dependent)
│
├── README.md              # Project documentation
│
└── LICENSE                # MIT license, if included
```

The `data/` directory and `items.txt` file are created or populated by the application when it saves records. The executable and license entries above depend on what you include in your repository; compiled binaries do not have to be committed.

---

## ⚙️ How It Works

EduLens-style dashboard simplicity meets the fundamentals of C programming through a straightforward data flow.

### 1. Application Startup

The program creates a GTK application and builds the dashboard interface, including the sidebar, search bar, summary cards, report list, category chart, and recovery gauge.

It then attempts to load previously saved records from `data/items.txt`.

If the file contains no saved records and sample-data loading is enabled, the application inserts demonstration reports and saves them locally.

### 2. In-Memory Record Management

Each item is represented by a C structure containing:

| Field | Description |
|:--|:--|
| `id` | Unique numeric identifier assigned to the report |
| `name` | Name or description of the item |
| `category` | Item classification |
| `location` | Place where the item was lost or found |
| `status` | Current report status |

All records are maintained in the array `items[MAX_ITEMS]`, with a maximum capacity of 500 entries.

### 3. Search and Filtering

Whenever the search query or a sidebar filter changes, the application refreshes the visible report list.

The filtering process checks:

- Selected status.
- Selected item category.
- Search matches in the item name, category, or location.

Search matching is case-insensitive, and the list displays newer records before older ones. A dedicated empty state appears when no records match the current search and filters.

### 4. Adding, Claiming, and Deleting Reports

- **Add:** Validate the item name and location, check the storage limit, assign a new ID, and append the report.
- **Claim:** Change the selected report's status to `Claimed`.
- **Delete:** Remove the selected record and shift subsequent array entries to fill the gap.

After each successful modification, the application saves the updated records and refreshes the dashboard.

### 5. Automatic Statistics Updates

The statistics function recalculates status totals and category counts from the current in-memory array.

The dashboard then updates the summary cards, sidebar counters, category bars, recovery percentage, and chart legend. This keeps the displayed analytics synchronized with the report data.

### 6. Local Data Persistence

Records are stored as tab-separated lines in:

```text
data/items.txt
```

The application writes the ID, name, category, location, and status for each record. On a subsequent launch, it reads these records back into memory.

This approach avoids requiring an external database server and keeps the project focused on C structures, arrays, functions, and file handling.

---

## 🧪 Sample Data & Configuration

The source code includes a set of sample reports representing common campus belongings, such as calculators, backpacks, textbooks, student ID cards, headphones, and water bottles.

Sample-data loading is controlled by the following compile-time setting in `main.c`:

```c
#define LOAD_SAMPLE_DATA 1
```

- Set it to `1` to populate an empty dataset with demonstration reports.
- Set it to `0` to leave an empty dataset when no saved records are available.

Sample reports are loaded only when the application has no records to load. Existing saved records are not intentionally replaced with the sample dataset during normal startup.

---

## 🔐 Data Handling & Validation

The application uses lightweight local storage and several checks to help maintain valid report records.

- **Required fields:** Item names and locations cannot be empty or consist entirely of spaces.
- **Length validation:** Names and locations must fit within their allocated character arrays.
- **Storage capacity:** New records are rejected when the 500-item limit is reached.
- **File format:** Tabs and line breaks are rejected in the required text fields to avoid disrupting the tab-separated record format.
- **Local persistence:** Successful record changes are written to the local data file.

**Storage note:** This is a local desktop application, not a networked multi-user system. The text file is not encrypted, and the application does not implement user authentication or role-based access control.

---

## 🎨 Interface Design

The interface uses a consistent royal-blue visual identity, with light backgrounds, rounded white panels, clear typography, and distinct status colors.

Key styling details include:

- **Navigation sidebar:** Status filters, category filters, item counts, and a prominent report shortcut.
- **Search toolbar:** A search field for item names, categories, and locations.
- **Summary cards:** Large numerical indicators for the current report totals.
- **Report cards:** Item icons, report IDs, category labels, locations, and status badges.
- **Analytics panels:** A category bar chart and custom recovery gauge.
- **Interactive controls:** Styled buttons, hover states, and modal dialogs for adding records and displaying validation messages.
- **Scrolling report area:** The item list scrolls independently to keep the dashboard controls and analytics accessible.

The stylesheet is loaded through GTK's CSS provider, keeping the interface's visual design separate from the main application logic.

---

## 🔮 Future Enhancements

Possible directions for extending the project include:

- 🔎 More advanced filtering and sorting options.
- 🖼️ Image attachments for reported belongings.
- 📤 Exporting reports to CSV or PDF.
- 🔔 Notifications for newly found or unclaimed items.
- 🗄️ SQLite integration for more structured data storage.
- 🔐 User authentication and administrator access.
- 📅 Date and time tracking for when reports are created or resolved.
- 📱 A more adaptive layout for smaller screens.

These are ideas for future versions and are not features currently implemented in the application.

---

## 📄 License

This project is distributed under the **MIT License**. Refer to the [`LICENSE`](LICENSE) file for the complete license terms if that file is included in the repository.

---

<p align="center">
  <strong>🏫 Campus Lost & Found Manager</strong>
  <br>
  <em>Making campus item recovery simpler, one report at a time.</em>
  <br><br>
  Built with C • GTK 4 • Cairo • GTK CSS
</p>
