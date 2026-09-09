// Run with: node --test '.github/scripts/*.test.js'

const test = require('node:test');
const assert = require('node:assert/strict');
const { PROJECT_ID, referencedIssues, findProjectItem, setFieldValue } = require('./project.js');

const item = (projectId, id = 'ITEM') => ({ id, project: { id: projectId } });
const projectItems = (...items) => ({ node: { projectItems: { nodes: items } } });

function fakeGithub(responses) {
  const calls = [];
  return {
    calls,
    graphql: async (query, variables) => {
      calls.push({ query, variables });
      const next = responses.shift();
      if (next instanceof Error) {
        throw next;
      }
      return next;
    },
  };
}

test('referencedIssues matches every closing keyword form, case-insensitively', () => {
  const body = 'Closes #1, closed #2, close #3. Fixes #4 fixed #5 fix #6. Resolves #7 resolved #8 resolve #9.';
  assert.deepEqual(referencedIssues(body), [1, 2, 3, 4, 5, 6, 7, 8, 9]);
  assert.deepEqual(referencedIssues('CLOSES #10'), [10]);
});

test('referencedIssues dedupes and needs a word boundary before the keyword', () => {
  assert.deepEqual(referencedIssues('Closes #5 and fixes #5'), [5]);
  assert.deepEqual(referencedIssues('encloses #123'), []);
  assert.deepEqual(referencedIssues('Closes#7'), []);
});

test('referencedIssues ignores keywords inside HTML comments', () => {
  const template = '## Related issues\n\n<!-- Closes #12, Relates to #34 -->\n\nCloses #60\n';
  assert.deepEqual(referencedIssues(template), [60]);
  assert.deepEqual(referencedIssues('<!-- multi\nline\nCloses #1 -->'), []);
});

test('referencedIssues tolerates a null or empty body', () => {
  assert.deepEqual(referencedIssues(null), []);
  assert.deepEqual(referencedIssues(undefined), []);
  assert.deepEqual(referencedIssues(''), []);
});

test('referencedIssues finds nothing in the release PR template', () => {
  const release = '## Release v0.2.0\n\n## Changes\n\n- #52 Add PR merge workflow\n- #62 Auto-set end date\n';
  assert.deepEqual(referencedIssues(release), []);
});

test('findProjectItem returns the item in the Hallmark project on the first try', async () => {
  const github = fakeGithub([projectItems(item('OTHER', 'X'), item(PROJECT_ID, 'HIT'))]);
  const found = await findProjectItem(github, 'NODE');
  assert.equal(found.id, 'HIT');
  assert.equal(github.calls.length, 1);
  assert.equal(github.calls[0].variables.nodeId, 'NODE');
});

test('findProjectItem returns null when the node is in no matching project', async () => {
  const github = fakeGithub([projectItems(item('OTHER'))]);
  assert.equal(await findProjectItem(github, 'NODE'), null);
});

test('findProjectItem retries until the item appears', async () => {
  const github = fakeGithub([projectItems(), projectItems(), projectItems(item(PROJECT_ID, 'LATE'))]);
  const found = await findProjectItem(github, 'NODE', { attempts: 3, baseDelayMs: 1 });
  assert.equal(found.id, 'LATE');
  assert.equal(github.calls.length, 3);
});

test('findProjectItem retries past a transient error and succeeds', async () => {
  const github = fakeGithub([new Error('502'), projectItems(item(PROJECT_ID, 'OK'))]);
  const found = await findProjectItem(github, 'NODE', { attempts: 2, baseDelayMs: 1 });
  assert.equal(found.id, 'OK');
});

test('findProjectItem fails fast on a non-transient error instead of retrying', async () => {
  const unauthorized = Object.assign(new Error('Bad credentials'), { status: 401 });
  const github = fakeGithub([unauthorized, projectItems(item(PROJECT_ID, 'NEVER'))]);
  await assert.rejects(findProjectItem(github, 'NODE', { attempts: 5 }), /Bad credentials/);
  assert.equal(github.calls.length, 1);
});

test('findProjectItem retries a transient status error', async () => {
  const gateway = Object.assign(new Error('Bad gateway'), { status: 502 });
  const github = fakeGithub([gateway, projectItems(item(PROJECT_ID, 'OK'))]);
  const found = await findProjectItem(github, 'NODE', { attempts: 2, baseDelayMs: 1 });
  assert.equal(found.id, 'OK');
  assert.equal(github.calls.length, 2);
});

test('findProjectItem throws the last error when every attempt errors', async () => {
  const github = fakeGithub([new Error('first'), new Error('last')]);
  await assert.rejects(findProjectItem(github, 'NODE', { attempts: 2, baseDelayMs: 1 }), /last/);
});

test('findProjectItem returns null when the final attempt is a clean miss after an error', async () => {
  const github = fakeGithub([new Error('blip'), projectItems()]);
  assert.equal(await findProjectItem(github, 'NODE', { attempts: 2, baseDelayMs: 1 }), null);
});

test('findProjectItem tolerates a null GraphQL payload', async () => {
  const github = fakeGithub([{ node: null }]);
  assert.equal(await findProjectItem(github, 'NODE'), null);
});

test('setFieldValue sends the project, item, field, and value as variables', async () => {
  const github = fakeGithub([{}]);
  await setFieldValue(github, 'ITEM', 'FIELD', { date: '2026-09-02' });
  assert.deepEqual(github.calls[0].variables, {
    projectId: PROJECT_ID,
    itemId: 'ITEM',
    fieldId: 'FIELD',
    value: { date: '2026-09-02' },
  });
  assert.match(github.calls[0].query, /updateProjectV2ItemFieldValue/);
});
