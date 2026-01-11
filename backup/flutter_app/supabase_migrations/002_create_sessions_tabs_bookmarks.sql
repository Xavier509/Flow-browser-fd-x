-- Supabase migrations: create sessions, tabs, and bookmarks tables

-- Sessions table: store serialized workspaces/sessions per user
CREATE TABLE IF NOT EXISTS public.sessions (
  id uuid DEFAULT gen_random_uuid() PRIMARY KEY,
  user_id text NOT NULL,
  name text NOT NULL DEFAULT 'default',
  workspaces jsonb,
  created_at timestamptz NOT NULL DEFAULT now(),
  updated_at timestamptz NOT NULL DEFAULT now()
);
CREATE INDEX IF NOT EXISTS idx_sessions_user ON public.sessions (user_id);

-- Tabs table: individual tab metadata for syncing
CREATE TABLE IF NOT EXISTS public.tabs (
  id uuid DEFAULT gen_random_uuid() PRIMARY KEY,
  user_id text NOT NULL,
  workspace text,
  tab_index integer,
  url text,
  title text,
  history jsonb,
  is_active boolean DEFAULT false,
  created_at timestamptz NOT NULL DEFAULT now(),
  updated_at timestamptz NOT NULL DEFAULT now()
);
CREATE INDEX IF NOT EXISTS idx_tabs_user ON public.tabs (user_id);

-- Bookmarks table: per-user bookmarks
CREATE TABLE IF NOT EXISTS public.bookmarks (
  id uuid DEFAULT gen_random_uuid() PRIMARY KEY,
  user_id text NOT NULL,
  url text NOT NULL,
  title text,
  favicon text,
  workspace text,
  created_at timestamptz NOT NULL DEFAULT now()
);
CREATE INDEX IF NOT EXISTS idx_bookmarks_user ON public.bookmarks (user_id);
