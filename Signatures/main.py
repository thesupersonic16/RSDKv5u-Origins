# oooo, it's the Windows~ ghost
import msvcrt
import os
from typing import List, Tuple

class FileHelper:
    file = None

    def __init__(self, path, mode: str):
        self.file = open(path, mode)
    
    def close(self):
        self.file.close()

    def readInt8(self):
        return int.from_bytes(self.file.read(1), "little")

    def readInt32(self):
        return int.from_bytes(self.file.read(4), "little")

    def readInt64(self):
        return int.from_bytes(self.file.read(8), "little")

    def readString(self, length: int = -1):
        if length == -1:
            length = self.readInt8()
        return self.file.read(length).decode("utf-8")

    def readBytes(self):
        length = self.readInt8()
        return self.file.read(length)

    def writeInt8(self, value: int):
        self.file.write(value.to_bytes(1, "little"))
    
    def writeInt32(self, value: int):
        self.file.write(value.to_bytes(4, "little"))

    def writeInt64(self, value: int):
        self.file.write(value.to_bytes(8, "little"))

    def writeString(self, value: str, writeLength: bool = True):
        if writeLength:
            self.writeInt8(len(value))
        self.file.write(value.encode("utf-8"))

    def writeBytes(self, value: bytes):
        self.writeInt8(len(value))
        self.file.write(value)


class SigScan:
    version: int = 1
    name: str = None
    pattern: bytes = None
    mask: str = None
    offset: int = 0
    type: int = 0
    hints: List[int] = None

    def __init__(self):
        self.hints = []

    def readObject(self, reader: FileHelper):
        self.version = reader.readInt8()
        self.name = reader.readString()
        self.pattern = reader.readBytes()
        self.mask = reader.readString(length=len(self.pattern))
        self.offset = reader.readInt32()
        self.type = reader.readInt8()
        hintsCount = reader.readInt8()
        self.hints = [reader.readInt64() for _ in range(hintsCount)]     
    
    def writeObject(self, writer: FileHelper):
        writer.writeInt8(self.version)
        writer.writeString(self.name)
        writer.writeBytes(self.pattern)
        writer.writeString(self.mask, writeLength=False)
        writer.writeInt32(self.offset)
        writer.writeInt8(self.type)
        writer.writeInt8(len(self.hints))
        for hint in self.hints:
            writer.writeInt64(hint)

class ScanFile:
    scans: List[SigScan] = []

    def load(self, path: str):
        reader = FileHelper(path, "rb")
        scansCount = reader.readInt8()
        for _ in range(scansCount):
            scan = SigScan()
            scan.readObject(reader)
            self.scans.append(scan)
        reader.close()
    
    def save(self, path: str):
        writer = FileHelper(path, "wb")
        writer.writeInt8(len(self.scans))
        for scan in self.scans:
            scan.writeObject(writer)
        writer.close()

    def addScan(self, scan: SigScan):
        self.scans.append(scan)

    def removeScan(self, index: int):
        if index >= 0 or index < len(self.scans):
            self.scans.pop(index)

    def getClasses(self) -> List[str]:
        classes: List[str] = []
        for scan in self.scans:
            className = scan.name.split("::")[0]
            if className not in classes:
                classes.append(className)
        return classes
    
    def getScansByClass(self, className: str) -> List[SigScan]:
        return [scan for scan in self.scans if scan.name.startswith(className)]


class Menu:
    title: str = None
    options: List[str] = []
    selectedIndex: int = 0

    def __init__(self, title: str, options: List[str], selectedIndex: int = 0):
        self.title = title
        self.options = options
        self.selectedIndex = selectedIndex
    
    def addOption(self, option):
        self.options.append(option)
    
    def render(self):
        print("\033[H\033[J")
        print(self.title)
        for i, option in enumerate(self.options):
            print(f"  {option}")

    def updateCursor(self, index: int):
        if index < 0:
            index = len(self.options) - 1
        if index >= len(self.options):
            index = 0
        self.selectedIndex = index
        print(f"\033[{index + 2};0H")

    def show(self) -> int:
        self.render()
        self.updateCursor(self.selectedIndex)
        while True:
            key = msvcrt.getch()
            if key == b"\xe0" or key == b"\x00":
                key = msvcrt.getch()
                if key == b"H":
                    self.updateCursor(self.selectedIndex - 1)
                elif key == b"P":
                    self.updateCursor(self.selectedIndex + 1)
            elif key == b"\r":
                return self.selectedIndex

def ConvertPattern(plainPattern: str) -> Tuple[str, str]:
    pattern = b""
    mask = ""
    workingPattern = plainPattern.replace(" ", "")
    if workingPattern.startswith("\\x"):
        for i in range(0, len(workingPattern), 4):
            pattern += bytes([int(workingPattern[i + 2:i + 4], 16)])
    else:
        i = 0
        while i < len(workingPattern):
            if workingPattern[i] == "?":
                pattern += b"\x00"
                mask += "?"
                i += 1
            else:
                pattern += bytes([int(workingPattern[i:i + 2], 16)])
                mask += "x"
                i += 2

    return pattern, mask

def ConvertHexStringToInt(hexString: str) -> int:
    return int(hexString, 16)

def MenuScan(scan: SigScan, scans: List[SigScan]):
    while True:
        options: List[str] = []
        options.append(f"Name: {scan.name}")
        options.append(f"Pattern: [{len(scan.pattern)} bytes]")
        options.append(f"Offset: {scan.offset}")
        options.append(f"Hints: [{len(scan.hints)} addresses]")
        options.append(f"Delete")
        options.append(f"Back")
        menu = Menu(f"Scan Options: {scan.name}", options)
        choice = menu.show()
        print("\033[H\033[J", end="")
        if choice == 0:
            print(f"Current Name: {scan.name}")
            scan.name = input("Name: ")
        elif choice == 1:
            print("\033[H\033[J", end="")
            scan.pattern = input("Pattern: ")
            
            # Code Pattern
            if scan.pattern.startswith("\\x"):
                scan.mask = input("Mask: ")
            pattern, _ = ConvertPattern(scan.pattern)
            scan.pattern = pattern

        elif choice == 2:
            scan.offset = ConvertHexStringToInt(input("Offset: "))
        elif choice == 3:
            pass
        elif choice == 4:
            scans.remove(scan)
            return
        elif choice == 5:
            return

def DisplayScans(title: str, scans: List[SigScan]):
    while True:
        options: List[str] = []
        for scan in scans:
            options.append(scan.name)
        options.append("Back")
        menu = Menu(title, options)
        choice = menu.show()
        if choice == len(scans):
            return
        else:
            MenuScan(scans[choice], scans)

def MenuShowAll(scans: List[SigScan]):
    DisplayScans("Signatures", scans)

def MenuShowClasses(scanFile: ScanFile):
    classes: List[str] = scanFile.getClasses()
    options: List[str] = classes.copy()
    options.append("Back")
    menu: Menu = Menu("Classes", options)
    choice: int = menu.show()
    if choice == len(classes):
        return
    else:
        DisplayScans(f"Class: {classes[choice]}", scanFile.getScansByClass(classes[choice]))

def MenuAddScan(scanFile: ScanFile):
    print("\033[H\033[J", end="")
    scan = SigScan()
    scan.name = input("Name: ")
    scan.pattern = input("Pattern: ")
    
    # Code Pattern
    if scan.pattern.startswith("\\x"):
        scan.mask = input("Mask: ")
    pattern, _ = ConvertPattern(scan.pattern)
    scan.pattern = pattern

    scan.offset = ConvertHexStringToInt(input("Offset: "))
    while True:
        hint = input("Hint: ")
        if hint == "":
            break
        scan.hints.append(ConvertHexStringToInt(hint))
    scanFile.addScan(scan)

def main():
    global scanFile
    scanFile = ScanFile()

    if os.path.exists("scans.bin"):
        scanFile.load("scans.bin")
    
    while True:
        menu = Menu("Main Menu", ["Show All", "Show Classes", "Add Scan", "Save", "Exit"])
        choice = menu.show()
        if choice == 0:
            MenuShowAll(scanFile.scans)
        elif choice == 1:
            MenuShowClasses(scanFile)
        elif choice == 2:
            MenuAddScan(scanFile)
        elif choice == 3:
            scanFile.save("scans.bin")
        elif choice == 4:
            break

main()