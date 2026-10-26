# frozen_string_literal: true

##
# Irbox Jekyll Build Hook: Extreme Optimizer & Obfuscator
# Automatically triggers `scripts/optimize.js` on `post_write` when:
# 1. Running in production: `JEKYLL_ENV=production bundle exec jekyll build`
# 2. Running with flag: `OPTIMIZE=true bundle exec jekyll build`
# 3. Enabled in _config.yml: `optimize.auto_run: true`
##

Jekyll::Hooks.register :site, :post_write do |site|
  should_run = ENV['JEKYLL_ENV'] == 'production' ||
               ENV['OPTIMIZE'] == 'true' ||
               site.config.dig('optimize', 'auto_run') == true

  if should_run
    Jekyll.logger.info 'Optimizer:', 'Executing extreme obfuscation and minification pipeline...'
    script_path = File.join(site.source, 'scripts', 'optimize.js')
    dest_dir = site.dest

    if File.exist?(script_path)
      flags = ['--dir', dest_dir]
      flags << '--obfuscate-only' if site.config.dig('optimize', 'obfuscate_only') == true
      flags << '--minify-only' if site.config.dig('optimize', 'minify_only') == true
      flags << '--no-precompress' if site.config.dig('optimize', 'precompress') == false

      cmd = "node #{script_path} #{flags.join(' ')}"
      system(cmd)
    end
  end
end
