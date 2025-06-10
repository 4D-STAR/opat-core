/**
 * A helper class to read binary data from an ArrayBuffer.
 * It keeps track of the current position and handles reading different data types.
 */
class DataReader {
    /**
     * @param {ArrayBuffer} arrayBuffer The file content.
     */
    constructor(arrayBuffer) {
        this.dataView = new DataView(arrayBuffer);
        this.decoder = new TextDecoder();
        this.offset = 0;
    }

    seek(newOffset) {
        this.offset = newOffset;
    }

    readString(length) {
        const strBytes = new Uint8Array(this.dataView.buffer, this.offset, length);
        this.offset += length;
        // Trim null characters from the end
        const firstNull = strBytes.indexOf(0);
        const nonNullSlice = firstNull === -1 ? strBytes : strBytes.slice(0, firstNull);
        return this.decoder.decode(nonNullSlice);
    }

    readBytes(length) {
        const bytes = new Uint8Array(this.dataView.buffer, this.offset, length);
        this.offset += length;
        return bytes;
    }

    readUint8() {
        const value = this.dataView.getUint8(this.offset, true);
        this.offset += 1;
        return value;
    }

    readUint16() {
        const value = this.dataView.getUint16(this.offset, true);
        this.offset += 2;
        return value;
    }

    readUint32() {
        const value = this.dataView.getUint32(this.offset, true);
        this.offset += 4;
        return value;
    }

    readUint64() {
        const value = this.dataView.getBigUint64(this.offset, true);
        this.offset += 8;
        return value;
    }

    readFloat64() {
        const value = this.dataView.getFloat64(this.offset, true);
        this.offset += 8;
        return value;
    }

    readFloat64Array(length) {
        const byteLength = length * 8;
        const values = new Float64Array(this.dataView.buffer, this.offset, length);
        this.offset += byteLength;
        return values;
    }
}

/**
 * Corresponds to the Header struct in opatIO.h
 */
class Header {
    constructor() {
        this.magic = "";
        this.version = 0;
        this.numTables = 0;
        this.headerSize = 0;
        this.indexOffset = 0n; // BigInt
        this.creationDate = "";
        this.sourceInfo = "";
        this.comment = "";
        this.numIndex = 0;
        this.hashPrecision = 0;
        this.reserved = null;
    }

    static fromReader(reader) {
        const h = new Header();
        h.magic = reader.readString(4);
        h.version = reader.readUint16();
        h.numTables = reader.readUint32();
        h.headerSize = reader.readUint32();
        h.indexOffset = reader.readUint64();
        h.creationDate = reader.readString(16);
        h.sourceInfo = reader.readString(64);
        h.comment = reader.readString(128);
        h.numIndex = reader.readUint16();
        h.hashPrecision = reader.readUint8();
        h.reserved = reader.readBytes(23);
        return h;
    }
}


/**
 * Corresponds to the CardHeader struct in opatIO.h
 */
class CardHeader {
    constructor() {
        this.magic = "";
        this.numTables = 0;
        this.headerSize = 0;
        this.indexOffset = 0n;
        this.cardSize = 0n;
        this.comment = "";
        this.reserved = null;
    }

    static fromReader(reader) {
        const h = new CardHeader();
        h.magic = reader.readString(4);
        h.numTables = reader.readUint32();
        h.headerSize = reader.readUint32();
        h.indexOffset = reader.readUint64();
        h.cardSize = reader.readUint64();
        h.comment = reader.readString(128);
        h.reserved = reader.readBytes(100);
        return h;
    }
}


/**
 * Corresponds to the CardCatalogEntry struct in opatIO.h
 */
class CardCatalogEntry {
    constructor() {
        this.index = []; // This will be an array of numbers (FloatIndexVector)
        this.byteStart = 0n;
        this.byteEnd = 0n;
        this.sha256 = null; // Uint8Array
    }

    /**
     * In JS, we can use a stringified version of the index vector as a key for Maps.
     */
    getKey() {
        return JSON.stringify(this.index);
    }

    static fromReader(reader, numIndex) {
        const entry = new CardCatalogEntry();
        entry.index = Array.from(reader.readFloat64Array(numIndex));
        entry.byteStart = reader.readUint64();
        entry.byteEnd = reader.readUint64();
        entry.sha256 = reader.readBytes(32);
        return entry;
    }
}


/**
 * Corresponds to the TableIndexEntry struct in opatIO.h
 */
class TableIndexEntry {
    constructor() {
        this.tag = "";
        this.byteStart = 0n;
        this.byteEnd = 0n;
        this.numColumns = 0;
        this.numRows = 0;
        this.columnName = "";
        this.rowName = "";
        this.size = 0n; // Vector size of each cell
        this.reserved = null;
    }

    static fromReader(reader) {
        const entry = new TableIndexEntry();
        entry.tag = reader.readString(8);
        entry.byteStart = reader.readUint64();
        entry.byteEnd = reader.readUint64();
        entry.numColumns = reader.readUint16();
        entry.numRows = reader.readUint16();
        entry.columnName = reader.readString(8);
        entry.rowName = reader.readString(8);
        entry.size = reader.readUint64();
        entry.reserved = reader.readBytes(12);
        return entry;
    }
}


/**
 * Corresponds to the OPATTable struct in opatIO.h
 */
class OPATTable {
    constructor() {
        this.rowValues = new Float64Array(0);
        this.columnValues = new Float64Array(0);
        this.data = new Float64Array(0);
        this.N_R = 0;
        this.N_C = 0;
        this.m_vsize = 0;
    }

    /**
     * Gets the vector at a specific cell.
     * @param {number} row The row index.
     * @param {number} column The column index.
     * @returns {Float64Array} The vector data in the cell.
     */
    getData(row, column) {
        if (row >= this.N_R || column >= this.N_C) {
            throw new Error("Index out of range");
        }
        const startIndex = (row * this.N_C + column) * this.m_vsize;
        return this.data.subarray(startIndex, startIndex + this.m_vsize);
    }

    /**
     * Gets a single primitive value from a cell.
     * @param {number} row
     * @param {number} column
     * @param {number} zdepth The index within the cell's vector.
     */
    getValue(row, column, zdepth = 0) {
        if (row >= this.N_R || column >= this.N_C || zdepth >= this.m_vsize) {
            throw new Error("Index out of range");
        }
        const index = (row * this.N_C + column) * this.m_vsize + zdepth;
        return this.data[index];
    }

    static fromReader(reader, cardEntry, tableEntry) {
        const table = new OPATTable();
        table.N_R = tableEntry.numRows;
        table.N_C = tableEntry.numColumns;
        table.m_vsize = Number(tableEntry.size); // Convert BigInt to Number

        reader.seek(Number(cardEntry.byteStart) + Number(tableEntry.byteStart));

        table.rowValues = reader.readFloat64Array(table.N_R);
        table.columnValues = reader.readFloat64Array(table.N_C);

        const dataSize = table.N_R * table.N_C * table.m_vsize;
        table.data = reader.readFloat64Array(dataSize);

        return table;
    }
}

/**
 * Corresponds to the DataCard struct in opatIO.h
 */
class DataCard {
    constructor() {
        /** @type {CardHeader} */
        this.header = null;
        /** @type {Map<string, TableIndexEntry>} */
        this.tableIndex = new Map();
        /** @type {Map<string, OPATTable>} */
        this.tableData = new Map();
    }

    /**
     * Get a table by its tag name.
     * @param {string} tag The tag of the table to retrieve.
     * @returns {OPATTable | undefined}
     */
    get(tag) {
        return this.tableData.get(tag);
    }

    static fromReader(reader, cardCatalogEntry) {
        const card = new DataCard();
        reader.seek(Number(cardCatalogEntry.byteStart));

        // 1. Read Card Header
        card.header = CardHeader.fromReader(reader);

        // 2. Read Table Index
        reader.seek(Number(cardCatalogEntry.byteStart) + Number(card.header.indexOffset));
        for (let i = 0; i < card.header.numTables; i++) {
            const tableIndexEntry = TableIndexEntry.fromReader(reader);
            card.tableIndex.set(tableIndexEntry.tag, tableIndexEntry);
        }

        // 3. Read all Table Data within this card
        for (const [tag, tableEntry] of card.tableIndex.entries()) {
            const table = OPATTable.fromReader(reader, cardCatalogEntry, tableEntry);
            card.tableData.set(tag, table);
        }

        return card;
    }
}


/**
 * The main class representing an entire OPAT file.
 */
class OPAT {
    constructor() {
        /** @type {Header} */
        this.header = null;
        /** @type {Map<string, CardCatalogEntry>} */
        this.cardCatalog = new Map();
        /** @type {Map<string, DataCard>} */
        this.cards = new Map();
    }

    /**
     * Get a DataCard by its index vector.
     * @param {number[]} index The index vector (e.g., [1.0, 2.5]).
     * @returns {DataCard | undefined}
     */
    get(index) {
        const key = JSON.stringify(index);
        return this.cards.get(key);
    }

    /**
     * Calculates and returns the bounds for each dimension.
     * @returns {Array<{min: number, max: number}>}
     */
    getBounds() {
        const bounds = Array(this.header.numIndex).fill(null).map(() => ({
            min: Infinity,
            max: -Infinity
        }));

        for (const entry of this.cardCatalog.values()) {
            for (let i = 0; i < this.header.numIndex; i++) {
                bounds[i].min = Math.min(bounds[i].min, entry.index[i]);
                bounds[i].max = Math.max(bounds[i].max, entry.index[i]);
            }
        }
        return bounds;
    }
}

/**
 * Main parsing function. Reads an ArrayBuffer and returns an OPAT object.
 * @param {ArrayBuffer} arrayBuffer The content of the .opat file.
 * @returns {OPAT} The parsed OPAT object.
 */
function parseOPAT(arrayBuffer) {
    const reader = new DataReader(arrayBuffer);
    const opat = new OPAT();

    // 1. Read Header
    opat.header = Header.fromReader(reader);
    if (opat.header.magic !== "OPAT") {
        throw new Error("File is not a valid OPAT file.");
    }

    // 2. Read Card Catalog
    reader.seek(Number(opat.header.indexOffset)); // Seek to where the catalog starts
    for (let i = 0; i < opat.header.numTables; i++) {
        const entry = CardCatalogEntry.fromReader(reader, opat.header.numIndex);
        opat.cardCatalog.set(entry.getKey(), entry);
    }

    // 3. Read all DataCards
    for (const entry of opat.cardCatalog.values()) {
        const dataCard = DataCard.fromReader(reader, entry);
        opat.cards.set(entry.getKey(), dataCard);
    }

    return opat;
}