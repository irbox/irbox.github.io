// anki-parser.js
// Make sure sql.js and jszip are loaded in index.html before this script.

let SQL; // Will hold the sql.js database API

async function initSqlJs() {
    if (!SQL) {
        try {
            SQL = await initSqlJs({
                locateFile: file => `https://cdnjs.cloudflare.com/ajax/libs/sql.js/1.10.3/${file}`
            });
            console.log('sql.js initialized.');
        } catch (error) {
            console.error('Failed to initialize sql.js:', error);
            throw new Error('Could not load SQL.js library.');
        }
    }
    return SQL;
}

/**
 * Parses an Anki .apkg file (which is a zip) and extracts Q&A pairs from the notes table.
 * Assumes a basic note structure where the first field is the question and the second is the answer.
 * @param {ArrayBuffer} fileBuffer - The ArrayBuffer of the .apkg file.
 * @returns {Promise<Array<{question: string, answer: string}>>} - A promise resolving to an array of Q&A objects.
 */
async function parseAnkiDeck(fileBuffer) {
    console.log('Starting Anki deck parsing...');
    const qaPairs = [];

    try {
        const jszip = new JSZip();
        const zip = await jszip.loadAsync(fileBuffer);
        console.log('Zip file loaded.');

        const dbFile = zip.file('collection.anki2') || zip.file('collection.anki21'); // Anki uses either
        if (!dbFile) {
            throw new Error('Could not find collection.anki2 or collection.anki21 in the Anki deck.');
        }

        const dbContent = await dbFile.async('uint8array');
        console.log('Database file extracted.');

        const SQL = await initSqlJs();
        const db = new SQL.Database(dbContent);
        console.log('SQLite database loaded.');

        // Query the 'notes' table
        // flds contains the fields separated by 0x1f (Unit Separator)
        // Check Anki schema for more details: https://docs.ankiweb.net/database.html#notes
        const res = db.exec("SELECT flds FROM notes");
        db.close(); // Close the database connection

        if (res.length > 0 && res[0].values) {
            for (const row of res[0].values) {
                const flds = row[0].split('\x1f'); // Split fields by Unit Separator
                if (flds.length >= 2) { // Ensure there are at least two fields (Front, Back)
                    // Basic heuristic: first field is question, second is answer
                    // Anki stores HTML, so we might want to strip it for embedding, but keep for display.
                    const questionHtml = flds[0] || '';
                    const answerHtml = flds[1] || '';

                    // Simple HTML stripping for embedding. Keep HTML for display.
                    const questionText = questionHtml.replace(/<[^>]*>/g, '').trim();
                    const answerText = answerHtml.replace(/<[^>]*>/g, '').trim();

                    if (questionText && answerText) {
                        qaPairs.push({
                            question: questionText,
                            answer: answerHtml // Store original HTML for display
                        });
                    }
                }
            }
        }
        console.log(`Found ${qaPairs.length} Q&A pairs.`);
        return qaPairs;

    } catch (error) {
        console.error('Error parsing Anki deck:', error);
        throw new Error(`Failed to parse Anki deck: ${error.message}`);
    }
}

export { parseAnkiDeck };
