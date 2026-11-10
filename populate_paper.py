import zipfile
import shutil
import os
import xml.etree.ElementTree as ET

# Namespaces
W_NS = "http://purl.oclc.org/ooxml/wordprocessingml/main"
W14_NS = "http://schemas.microsoft.com/office/word/2010/wordml"
REL_NS = "http://purl.oclc.org/ooxml/officeDocument/relationships"

ET.register_namespace('w', W_NS)
ET.register_namespace('w14', W14_NS)
ET.register_namespace('r', REL_NS)

def make_el(tag, text=None, attrib=None):
    el = ET.Element(f"{{{W_NS}}}{tag}", attrib or {})
    if text:
        el.text = text
    return el

def make_r(text, bold=False, italic=False, size=None, preserve_space=False, small_caps=False):
    r = make_el("r")
    rPr = make_el("rPr")
    has_rPr = False
    if bold:
        rPr.append(make_el("b"))
        has_rPr = True
    if italic:
        rPr.append(make_el("i"))
        has_rPr = True
    if small_caps:
        rPr.append(make_el("smallCaps"))
        has_rPr = True
    if size:
        rPr.append(make_el("sz", attrib={f"{{{W_NS}}}val": str(size)}))
        rPr.append(make_el("szCs", attrib={f"{{{W_NS}}}val": str(size)}))
        has_rPr = True
    if has_rPr:
        r.append(rPr)
    
    t_attrib = {}
    if preserve_space or (text and (text.startswith(" ") or text.endswith(" "))):
        t_attrib["{http://www.w3.org/XML/1998/namespace}space"] = "preserve"
    t = make_el("t", text=text, attrib=t_attrib)
    r.append(t)
    return r

def make_br():
    r = make_el("r")
    r.append(make_el("br"))
    return r

def make_para(style_name, runs):
    p = make_el("p")
    pPr = make_el("pPr")
    if style_name:
        pPr.append(make_el("pStyle", attrib={f"{{{W_NS}}}val": style_name}))
    p.append(pPr)
    for item in runs:
        if isinstance(item, ET.Element):
            p.append(item)
    return p

def make_heading(level, text):
    style = f"Heading{level}"
    return make_para(style, [make_r(text)])

def make_body_p(text):
    return make_para("BodyText", [make_r(text)])

def make_bullet_p(bold_prefix, text):
    return make_para("bulletlist", [
        make_r(bold_prefix, bold=True),
        make_r(" " + text, preserve_space=True)
    ])

def make_ref_p(index_str, authors, title, publication):
    p = make_el("p")
    pPr = make_el("pPr")
    pPr.append(make_el("pStyle", attrib={f"{{{W_NS}}}val": "references"}))
    pPr.append(make_el("ind", attrib={f"{{{W_NS}}}start": "17.70pt", f"{{{W_NS}}}hanging": "17.70pt"}))
    p.append(pPr)
    p.append(make_r(f"[{index_str}]  "))
    p.append(make_r(authors + ", "))
    p.append(make_r(f"\"{title},\" "))
    p.append(make_r(publication, italic=True))
    p.append(make_r("."))
    return p

def build_table():
    tbl = make_el("tbl")
    tblPr = make_el("tblPr")
    tblPr.append(make_el("tblW", attrib={f"{{{W_NS}}}w": "0pt", f"{{{W_NS}}}type": "dxa"}))
    tblPr.append(make_el("jc", attrib={f"{{{W_NS}}}val": "center"}))
    borders = make_el("tblBorders")
    for b_name in ["top", "start", "bottom", "end", "insideH", "insideV"]:
        borders.append(make_el(b_name, attrib={
            f"{{{W_NS}}}val": "single",
            f"{{{W_NS}}}sz": "2",
            f"{{{W_NS}}}space": "0",
            f"{{{W_NS}}}color": "auto"
        }))
    tblPr.append(borders)
    tblPr.append(make_el("tblLayout", attrib={f"{{{W_NS}}}type": "fixed"}))
    tbl.append(tblPr)

    # Grid columns
    tblGrid = make_el("tblGrid")
    for col_w in ["1200", "1200", "1200", "1200"]:
        tblGrid.append(make_el("gridCol", attrib={f"{{{W_NS}}}w": col_w}))
    tbl.append(tblGrid)

    # Header Row
    headers = ["Test Input File", "Original Size", "Compressed (.huf)", "Space Saved"]
    tr0 = make_el("tr")
    trPr0 = make_el("trPr")
    trPr0.append(make_el("tblHeader"))
    tr0.append(trPr0)
    for h in headers:
        tc = make_el("tc")
        tcPr = make_el("tcPr")
        tcPr.append(make_el("tcW", attrib={f"{{{W_NS}}}w": "60pt", f"{{{W_NS}}}type": "dxa"}))
        tc.append(tcPr)
        p = make_el("p")
        pPr = make_el("pPr")
        pPr.append(make_el("pStyle", attrib={f"{{{W_NS}}}val": "tablecolhead"}))
        p.append(pPr)
        p.append(make_r(h, bold=True))
        tc.append(p)
        tr0.append(tc)
    tbl.append(tr0)

    # Data Rows
    data = [
        ("empty_file.txt", "0 B", "14 B", "0.0% (header)"),
        ("english_text.txt", "365 B", "259 B", "29.0%"),
        ("sample.txt", "1,553 B", "1,000 B", "35.6%"),
        ("single_char.txt (500 'Z')", "500 B", "78 B", "84.4%"),
        ("all_256_bytes.txt", "2,560 B", "2,894 B", "-13.0% (overhead)")
    ]

    for row in data:
        tr = make_el("tr")
        for cell_val in row:
            tc = make_el("tc")
            tcPr = make_el("tcPr")
            tcPr.append(make_el("tcW", attrib={f"{{{W_NS}}}w": "60pt", f"{{{W_NS}}}type": "dxa"}))
            tc.append(tcPr)
            p = make_el("p")
            pPr = make_el("pPr")
            pPr.append(make_el("pStyle", attrib={f"{{{W_NS}}}val": "tablecopy"}))
            p.append(pPr)
            p.append(make_r(cell_val))
            tc.append(p)
            tr.append(tc)
        tbl.append(tr)

    return tbl

def generate_new_body(old_p3, old_p8, old_p90, old_sectPr):
    body = make_el("body")

    # P0: Title
    p0 = make_el("p")
    p0_pPr = make_el("pPr")
    p0_pPr.append(make_el("pStyle", attrib={f"{{{W_NS}}}val": "papertitle"}))
    p0_pPr.append(make_el("spacing", attrib={
        f"{{{W_NS}}}before": "5pt",
        f"{{{W_NS}}}beforeAutospacing": "1",
        f"{{{W_NS}}}after": "5pt",
        f"{{{W_NS}}}afterAutospacing": "1"
    }))
    p0.append(p0_pPr)
    p0.append(make_r("Implementation of a Lossless Bit-Level Huffman File Compressor for Operating Systems File Systems"))
    body.append(p0)

    # P1: Subtitle note (or course note)
    p1 = make_el("p")
    p1_pPr = make_el("pPr")
    p1_pPr.append(make_el("pStyle", attrib={f"{{{W_NS}}}val": "Author"}))
    p1.append(p1_pPr)
    p1.append(make_r("Course Project: Operating Systems (COCSC301)", italic=True, size=18))
    body.append(p1)

    # Empty spacer paragraph P2
    body.append(make_para("Normal", []))

    # P3: Author Section Break (preserves 1-column layout for title and author blocks)
    body.append(old_p3)

    # P4: Author 1 (Siddhartha Pal)
    p_auth1 = make_el("p")
    p_auth1_pPr = make_el("pPr")
    p_auth1_pPr.append(make_el("pStyle", attrib={f"{{{W_NS}}}val": "Author"}))
    p_auth1_pPr.append(make_el("spacing", attrib={f"{{{W_NS}}}before": "5pt", f"{{{W_NS}}}beforeAutospacing": "1"}))
    p_auth1.append(p_auth1_pPr)
    p_auth1.append(make_r("Siddhartha Pal", bold=True, size=20))
    p_auth1.append(make_br())
    p_auth1.append(make_r("Dept. of Computer Science and Engineering", size=18))
    p_auth1.append(make_br())
    p_auth1.append(make_r("Netaji Subhas University of Technology", size=18))
    p_auth1.append(make_br())
    p_auth1.append(make_r("New Delhi, India", size=18))
    p_auth1.append(make_br())
    p_auth1.append(make_r("sp8k2007@gmail.com", size=18))
    body.append(p_auth1)

    # P5: Author 2 (Zeeshan Ahmad Ansari)
    p_auth2 = make_el("p")
    p_auth2_pPr = make_el("pPr")
    p_auth2_pPr.append(make_el("pStyle", attrib={f"{{{W_NS}}}val": "Author"}))
    p_auth2_pPr.append(make_el("spacing", attrib={f"{{{W_NS}}}before": "5pt", f"{{{W_NS}}}beforeAutospacing": "1"}))
    p_auth2.append(p_auth2_pPr)
    p_auth2.append(make_r("Zeeshan Ahmad Ansari", bold=True, size=20))
    p_auth2.append(make_br())
    p_auth2.append(make_r("Dept. of Computer Science and Engineering", size=18))
    p_auth2.append(make_br())
    p_auth2.append(make_r("Netaji Subhas University of Technology", size=18))
    p_auth2.append(make_br())
    p_auth2.append(make_r("New Delhi, India", size=18))
    p_auth2.append(make_br())
    p_auth2.append(make_r("zeeshanahmad2563@gmail.com", size=18))
    body.append(p_auth2)

    # Update old_p8 section break cols from 3 to 2 for the 2-author layout
    pPr8 = old_p8.find(f"{{{W_NS}}}pPr")
    if pPr8 is not None:
        sectPr8 = pPr8.find(f"{{{W_NS}}}sectPr")
        if sectPr8 is not None:
            cols8 = sectPr8.find(f"{{{W_NS}}}cols")
            if cols8 is not None:
                cols8.attrib[f"{{{W_NS}}}num"] = "2"
    body.append(old_p8)

    # Abstract Paragraph
    p_abs = make_el("p")
    p_abs_pPr = make_el("pPr")
    p_abs_pPr.append(make_el("pStyle", attrib={f"{{{W_NS}}}val": "Abstract"}))
    p_abs.append(p_abs_pPr)
    p_abs.append(make_r("Abstract", bold=True, italic=True))
    p_abs.append(make_r("—File compression plays an essential role in modern operating systems by reducing secondary storage utilization and lowering transmission overhead across input/output channels. In standard ASCII and UTF-8 encodings, every symbol occupies a fixed width of eight bits, regardless of its frequency of occurrence. In this paper, we present the design and implementation of a standalone, lossless file compressor developed in modern C++17 based on the classic Huffman coding algorithm. Our implementation features a two-pass architecture, a deterministic min-heap priority queue for prefix-tree generation, and custom bit-level stream primitives (BitWriter and BitReader) that pack variable-length prefix codes directly into binary bytes. To ensure files remain fully self-contained, a compact pre-order representation of the tree topology along with original byte counts are serialized into a proprietary binary header (.huf). Empirical tests conducted on text documents, repetitive sequences, and extreme boundary cases demonstrate compression savings between 29% and 84%, with strict byte-for-byte fidelity upon decompression."))
    body.append(p_abs)

    # Keywords Paragraph
    p_kw = make_el("p")
    p_kw_pPr = make_el("pPr")
    p_kw_pPr.append(make_el("pStyle", attrib={f"{{{W_NS}}}val": "Keywords"}))
    p_kw.append(p_kw_pPr)
    p_kw.append(make_r("Keywords", bold=True, italic=True))
    p_kw.append(make_r("—lossless data compression, Huffman coding, binary prefix tree, bit-level I/O, operating systems, min-heap."))
    body.append(p_kw)

    # SECTION I: INTRODUCTION
    body.append(make_heading(1, "I. Introduction"))
    body.append(make_body_p("In computer operating systems, storage subsystems and virtual memory managers consistently handle large volumes of unstructured and structured data. Typical plain text files represent each character using standard eight-bit ASCII or UTF-8 formatting. Consequently, frequently used characters such as the letter 'e' or space (' ') consume the exact same eight bits of storage as rarely used symbols such as 'x' or 'z'. This fixed-width character representation creates inherent statistical redundancy."))
    body.append(make_body_p("Lossless data compression algorithms eliminate this redundancy without discarding any information, allowing the original bitstream to be recovered identically. Operating systems use lossless compression in file systems (such as ZFS and Btrfs), executable packers, archive utilities, and network sockets."))
    body.append(make_body_p("As part of our undergraduate coursework in Operating Systems (COCSC301), we engineered an end-to-end, high-performance command-line file compression utility in C++17. The program accepts any text file or entire directory from the user, computes character frequencies, constructs an optimal binary prefix tree, and writes a compressed binary file (.huf) located alongside the source document. This paper outlines the architecture of our implementation, the underlying data structures, bit-level streaming mechanisms, and empirical performance results."))

    # SECTION II: HUFFMAN CODING PRINCIPLES
    body.append(make_heading(1, "II. Huffman Coding Principles"))
    body.append(make_heading(2, "A. Variable-Length Prefix Codes"))
    body.append(make_body_p("Published by David Huffman in 1952, Huffman coding is a greedy algorithm that creates optimal prefix-free codes. A prefix code is defined such that no individual character code forms a prefix of any other character code. This property guarantees that an incoming sequence of variable-length bits can be decoded unambiguously from left to right without requiring explicit delimiter characters."))
    body.append(make_heading(2, "B. The Greedy Tree Construction Algorithm"))
    body.append(make_body_p("The Huffman algorithm operates by building a binary tree from bottom to top using the following steps:"))
    body.append(make_bullet_p("1. Frequency Counting:", "Count the frequency of each unique byte value (0 to 255) occurring in the source document."))
    body.append(make_bullet_p("2. Priority Queue Initialization:", "Instantiate a leaf node for each character with non-zero frequency and push all leaf nodes into a min-heap priority queue sorted in ascending order of frequency."))
    body.append(make_bullet_p("3. Subtree Merging:", "Extract the two nodes with the lowest frequencies (N1 and N2). Create an internal parent node whose frequency equals the sum of N1 and N2, attach N1 as the left child and N2 as the right child, and reinsert the parent node into the min-heap."))
    body.append(make_bullet_p("4. Termination:", "Repeat step 3 until exactly one node remains in the queue. This final node serves as the root of the complete Huffman tree."))
    body.append(make_body_p("By convention, traversing an edge to a left child appends a '0' bit, while traversing to a right child appends a '1' bit. As a natural outcome of this greedy strategy, symbols with high frequencies reside close to the root with short codes, whereas rare symbols reside deep in the tree with longer codes."))

    # SECTION III: SYSTEM ARCHITECTURE & IMPLEMENTATION
    body.append(make_heading(1, "III. System Architecture and Implementation"))
    body.append(make_body_p("Our software system is structured modularly across six distinct components written in C++17, designed to run on POSIX Linux environments."))

    body.append(make_heading(2, "A. Two-Pass Compression Pipeline"))
    body.append(make_body_p("Because Huffman coding requires knowledge of global character probabilities, single-pass processing is not possible without adaptive trees. Our system uses a clean two-pass pipeline:"))
    body.append(make_bullet_p("• Pass 1 (Frequency Analysis):", "The compressor reads the input file in 64-kilobyte buffered blocks and tallies the byte counts across an array of 256 unsigned 64-bit integers. The total byte length and count of distinct characters are determined."))
    body.append(make_bullet_p("• Header Writing & Code Generation:", "The min-heap builds the Huffman tree, extracts prefix codes via depth-first search, and serializes the tree structure into the output stream."))
    body.append(make_bullet_p("• Pass 2 (Payload Encoding):", "The input file stream is rewound to byte offset zero using seekg(). The compressor re-reads the source bytes, looks up each byte's prefix code string in a constant-time array lookup, and passes the code bits into BitWriter."))

    body.append(make_heading(2, "B. Bit-Level Stream Management (BitWriter & BitReader)"))
    body.append(make_body_p("Operating system read and write system calls exclusively manipulate memory in granular chunks of whole 8-bit bytes. However, individual Huffman codes are often 2, 4, or 7 bits long. To bridge this gap, we implemented BitWriter and BitReader:"))
    body.append(make_body_p("BitWriter maintains an internal 8-bit buffer and a bit-counter. When a bit is added, the buffer shifts left by one position and inserts the new bit via a bitwise OR operation. Once eight bits accumulate, the byte is written to the standard output stream, and the buffer resets. When encoding finishes, flush() zero-pads any unfinished byte to align it with the byte boundary."))
    body.append(make_body_p("BitReader reverses this process: it reads an 8-bit byte from the disk stream, stores it in an internal buffer, and dispenses individual bits on demand by right-shifting and masking."))

    body.append(make_heading(2, "C. Self-Contained Binary Header Format"))
    body.append(make_body_p("A compressed file must be decompressible on any system without external metadata. We designed a compact, cross-platform file format:"))
    body.append(make_bullet_p("1. Magic Identifier (4 Bytes):", "The ASCII sequence 'H', 'U', 'F', followed by version byte 0x01, preventing inadvertent decompression of corrupted or foreign files."))
    body.append(make_bullet_p("2. Original File Size (8 Bytes):", "A 64-bit integer serialized in little-endian order. This prevents reading extraneous padding bits that may exist in the final byte of the bitstream."))
    body.append(make_bullet_p("3. Distinct Symbol Count (2 Bytes):", "A 16-bit integer recording the number of unique byte symbols."))
    body.append(make_bullet_p("4. Serialized Tree Topology:", "The tree is written using a pre-order traversal. For each internal node, a single '0' bit is emitted. For each leaf node, a '1' bit is emitted followed by the uncompressed 8-bit character byte. This encodes the entire tree structure in only a few dozen to a few hundred bytes."))

    body.append(make_heading(2, "D. Filesystem and Command-Line Interface"))
    body.append(make_body_p("Using the C++17 std::filesystem library, the utility handles both individual files and batch folder processing. When a path is supplied, the program inspects whether it is a regular file or a directory. In directory mode, it identifies all files matching the .txt extension and compresses each file in-place within that folder. An interactive prompt mode is also provided if the program is launched without CLI flags."))

    # SECTION IV: EXPERIMENTAL RESULTS
    body.append(make_heading(1, "IV. Experimental Results and Analysis"))
    body.append(make_body_p("To evaluate compression efficiency and correctness, we subjected our application to automated tests across diverse data distributions. Table I details the resulting metrics."))

    # Table caption and Table
    body.append(make_para("tablehead", [make_r("TABLE I.   EXPERIMENTAL COMPRESSION PERFORMANCE METRICS", bold=True, size=16)]))
    body.append(build_table())

    body.append(make_heading(2, "A. Analysis of Results"))
    body.append(make_body_p("As observed in Table I, typical English prose (such as sample.txt and english_text.txt) yields between 29% and 36% space savings. This occurs because the letter frequencies in natural language are non-uniform: high-frequency vowels and spaces are assigned compact 3-to-4 bit codes, significantly outperforming fixed 8-bit representations."))
    body.append(make_body_p("In the case of highly repetitive data (single_char.txt), the tree degenerates into a single node with code length 1, producing an 84.4% reduction in file size."))
    body.append(make_body_p("Conversely, when tested on a uniform distribution containing all 256 byte values in equal proportion (all_256_bytes.txt), Huffman coding produces negative savings (-13.0%). In information theory, a uniform distribution has maximum Shannon entropy, rendering statistical compression impossible. The resulting size increase reflects the storage cost of serializing the 256-node tree header."))
    body.append(make_body_p("Across all test suites, roundtrip decompression was verified through memory comparison (memcmp) against the original file, validating that zero data corruption occurred."))

    # SECTION V: CONCLUSION & FUTURE WORK
    body.append(make_heading(1, "V. Conclusion and Future Work"))
    body.append(make_body_p("In this project, we designed and implemented a complete, bit-level Huffman compression system in C++17 tailored for operating systems coursework. We demonstrated how theoretical computer science concepts—greedy algorithms, min-heaps, and binary trees—directly integrate with low-level systems programming concepts including bit-manipulation, block I/O buffering, endian-neutral serialization, and POSIX filesystem operations."))
    body.append(make_body_p("Future enhancements could combine Huffman coding with dictionary-based sliding-window algorithms (such as LZ77) to implement the full DEFLATE standard, as well as multi-threaded block compression using POSIX threads to exploit modern multi-core processors."))

    # ACKNOWLEDGMENT
    body.append(make_para("Heading5", [make_r("Acknowledgment", bold=True)]))
    body.append(make_body_p("The authors wish to thank the Department of Computer Science and Engineering at Netaji Subhas University of Technology, New Delhi, for providing the academic curriculum and computing facilities for the Operating Systems course (COCSC301)."))

    # REFERENCES
    body.append(make_para("Heading5", [make_r("References", bold=True)]))
    body.append(make_ref_p("1", "D. A. Huffman", "A Method for the Construction of Minimum-Redundancy Codes", "Proc. IRE, vol. 40, no. 9, pp. 1098–1101, Sept. 1952"))
    body.append(make_ref_p("2", "A. Silberschatz, P. B. Galvin, and G. Gagne", "Operating System Concepts, 10th ed.", "Hoboken, NJ: John Wiley & Sons, 2018"))
    body.append(make_ref_p("3", "T. H. Cormen, C. E. Leiserson, R. L. Rivest, and C. Stein", "Introduction to Algorithms, 3rd ed.", "Cambridge, MA: MIT Press, 2009"))
    body.append(make_ref_p("4", "C. E. Shannon", "A Mathematical Theory of Communication", "Bell System Technical Journal, vol. 27, no. 3, pp. 379–423, July 1948"))
    body.append(make_ref_p("5", "P. Deutsch", "DEFLATE Compressed Data Format Specification version 1.3", "IETF RFC 1951, May 1996"))
    body.append(make_ref_p("6", "B. Stroustrup", "The C++ Programming Language, 4th ed.", "Boston, MA: Addison-Wesley, 2013"))

    # Final spacer
    body.append(make_para("Normal", []))

    # Add the final 2-column section break from the original document
    body.append(old_p90)
    body.append(old_sectPr)

    return body

def main():
    print("Reading paper.docx ...")
    with zipfile.ZipFile("paper.docx", "r") as z_in:
        xml_content = z_in.read("word/document.xml")
        root = ET.fromstring(xml_content)
        body = list(root.iter(f"{{{W_NS}}}body"))[0]

        # Extract existing section break elements to preserve exact page layout
        old_p3 = body[3]
        old_p8 = body[8]
        old_p90 = body[90]
        old_sectPr = body[92]

        new_body = generate_new_body(old_p3, old_p8, old_p90, old_sectPr)
        
        # Replace the body in the XML tree
        root.remove(body)
        root.append(new_body)

        new_doc_xml = ET.tostring(root, encoding="utf-8", xml_declaration=True)

        print("Writing updated paper to paper_updated.docx ...")
        with zipfile.ZipFile("paper_updated.docx", "w", compression=zipfile.ZIP_DEFLATED) as z_out:
            for item in z_in.infolist():
                if item.filename == "word/document.xml":
                    z_out.writestr(item, new_doc_xml)
                else:
                    z_out.writestr(item, z_in.read(item.filename))

    print("Overwriting paper.docx with updated version ...")
    shutil.move("paper_updated.docx", "paper.docx")
    print("Successfully populated paper.docx!")

if __name__ == "__main__":
    main()

