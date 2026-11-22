import { readdir, readFile, writeFile } from 'node:fs/promises';
import { join, extname, relative } from 'node:path';
import { fileURLToPath } from 'node:url';

const ROOT = process.cwd();
const TARGET_DIR = ROOT;

const SKIP_DIRS = new Set(['node_modules', '.git', 'dist', 'build', 'coverage', '.next', '.cache']);

const TEXT_EXTS = new Set([
    '.cpp', '.hpp', '.h',
]);

const CRLF = '\r\n';
const LF = '\n';
const CR = '\r';

function toCRLF(text) {
    return text.replace(/\r\n|\r/g, LF).replace(/\n/g, CRLF);
}

function looksBinary(buffer) {
    const sample = buffer.subarray(0, 8000);
    for (let i = 0; i < sample.length; i++) {
        if (sample[i] === 0) return true;
    }
    return false;
}

function isTextCandidate(filePath) {
    const ext = extname(filePath).toLowerCase();
    if (ext) return TEXT_EXTS.has(ext);
    return false
}

async function collectFiles(dir, out = []) {
    let entries;
    try {
        entries = await readdir(dir, { withFileTypes: true });
    } catch (err) {
        throw err;
    }

    for (const entry of entries) {
        const full = join(dir, entry.name);
        if (entry.isDirectory()) {
            if (SKIP_DIRS.has(entry.name)) continue;
            await collectFiles(full, out);
        } else if (entry.isFile()) {
            out.push(full);
        }
    }
    return out;
}

async function main() {
    const checkOnly = process.argv.includes('--check');
    const files = await collectFiles(TARGET_DIR);

    let changed = 0;
    let scanned = 0;
    let skipped = 0;

    for (const file of files) {
        if (!isTextCandidate(file)) {
            skipped++;
            continue;
        }
        scanned++;

        const buf = await readFile(file);
        if (looksBinary(buf)) {
            skipped++;
            continue;
        }

        const original = buf.toString('utf8');
        const normalized = toCRLF(original);

        if (normalized === original) continue;

        const rel = relative(ROOT, file).split('\\').join('/');
        if (checkOnly) {
            console.log(`✗ ${rel}`);
        } else {
            await writeFile(file, normalized, 'utf8');
            console.log(`✓ ${rel}`);
        }
        changed++;
    }

    console.log('');
    console.log(`Scanned: ${scanned}  Skip: ${skipped}  ${checkOnly ? 'Bad' : 'Fucked'}: ${changed}`);

    if (checkOnly && changed > 0) {
        process.exitCode = 1;
    }
}

export default main

if (fileURLToPath(import.meta.url) == process.argv[1]) {
    await main()
}

