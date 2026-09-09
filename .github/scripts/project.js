// Shared GitHub Project helpers for actions/github-script steps.
//
// Load with:
//   const project = require(`${process.env.GITHUB_WORKSPACE}/.github/scripts/project.js`);
//
// `github` is the authenticated Octokit instance github-script injects.

// Hallmark project (users/flwrenn/projects/12). Every project identifier lives
// here so a re-created field is one edit, not a hunt through workflow env blocks.
const PROJECT_ID = 'PVT_kwHOBh2HNc4BPjfG';
const STATUS_FIELD_ID = 'PVTSSF_lAHOBh2HNc4BPjfGzg97gkw';
const PR_OPEN_OPTION_ID = 'df73e18b';
const END_DATE_FIELD_ID = 'PVTF_lAHOBh2HNc4BPjfGzg97glk';

const CLOSING_KEYWORD = /\b(?:close[sd]?|fix(?:e[sd])?|resolve[sd]?)\s+#(\d+)/gi;
const HTML_COMMENT = /<!--[\s\S]*?-->/g;

const PROJECT_ITEMS_QUERY = `
  query($nodeId: ID!) {
    node(id: $nodeId) {
      ... on Issue { projectItems(first: 10) { nodes { id project { id } } } }
      ... on PullRequest { projectItems(first: 10) { nodes { id project { id } } } }
    }
  }`;

const SET_FIELD_MUTATION = `
  mutation($projectId: ID!, $itemId: ID!, $fieldId: ID!, $value: ProjectV2FieldValue!) {
    updateProjectV2ItemFieldValue(input: {
      projectId: $projectId
      itemId: $itemId
      fieldId: $fieldId
      value: $value
    }) { projectV2Item { id } }
  }`;

const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

// Issue numbers named by a closing keyword in the visible PR body.
// Text inside HTML comments is ignored so template hints never close anything.
function referencedIssues(body) {
  const visible = (body || '').replace(HTML_COMMENT, '');
  const numbers = [...visible.matchAll(CLOSING_KEYWORD)].map((m) => parseInt(m[1], 10));
  return [...new Set(numbers)];
}

// A transport hiccup is worth another attempt. A 401, 403, or 404 is a token or
// permission problem that the next four attempts would hit identically, so it
// fails immediately rather than sleeping 45s first.
function isTransient(error) {
  if (!error?.status) {
    return true;
  }
  return [429, 502, 503, 504].includes(error.status);
}

// The project item for an issue or PR node, or null when it is not in the project.
// Retries cover the delay between an item being created and the built-in
// "auto-add to project" automation picking it up.
async function findProjectItem(github, nodeId, { attempts = 1, baseDelayMs = 3000 } = {}) {
  let lastError = null;
  for (let attempt = 1; attempt <= attempts; attempt++) {
    try {
      const result = await github.graphql(PROJECT_ITEMS_QUERY, { nodeId });
      const nodes = result?.node?.projectItems?.nodes ?? [];
      const item = nodes.find((n) => n?.project?.id === PROJECT_ID);
      if (item) {
        return item;
      }
      lastError = null;
    } catch (error) {
      if (!isTransient(error)) {
        throw error;
      }
      lastError = error;
    }
    if (attempt < attempts) {
      await sleep(baseDelayMs * 2 ** (attempt - 1));
    }
  }
  if (lastError) {
    throw lastError;
  }
  return null;
}

// `value` is a ProjectV2FieldValue: { date }, { singleSelectOptionId }, { text }, ...
async function setFieldValue(github, itemId, fieldId, value) {
  await github.graphql(SET_FIELD_MUTATION, { projectId: PROJECT_ID, itemId, fieldId, value });
}

module.exports = {
  PROJECT_ID,
  STATUS_FIELD_ID,
  PR_OPEN_OPTION_ID,
  END_DATE_FIELD_ID,
  referencedIssues,
  findProjectItem,
  setFieldValue,
};
