idf_component_register(
    EMBED_TXTFILES "public/index.html" "public/build/bundle.css" "public/build/bundle.js" 
    INCLUDE_DIRS ".")

# add_custom_command(OUTPUT "public/build/bundle.css" "public/build/bundle.js"
#     COMMAND cd ${COMPONENT_DIR} && npm run build
#     COMMENT "Generate Svelte site"
# )

# add_custom_target(svelte_bundle DEPENDS "public/build/bundle.css" "public/build/bundle.js")
# add_dependencies(${COMPONENT_LIB} svelte_bundle)